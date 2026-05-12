"""
Tiny HTTP server that backs the Compiler Playground.

Workflow:
  1. Serves playground.html at GET /
  2. Accepts POST /compile with JSON body {"source": "..."}
       - writes the source to a temp file
       - runs ./Compiler.exe on it
       - reads back the four output files
       - returns everything as JSON

No external dependencies (stdlib only). Run with:
    python playground_server.py
Then open http://localhost:5050 in any browser.
"""

import json
import os
import shutil
import subprocess
import sys
import tempfile
import webbrowser
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HERE = Path(__file__).resolve().parent
PORT = 5050
COMPILER = HERE / "Compiler.exe"
HTML_FILE = HERE / "playground.html"

# Preset example programs the UI shows in a dropdown.
EXAMPLES = {
    "hello": {
        "label": "Hello world (basics)",
        "source":
            'int a = 5;\n'
            'int b = 10;\n'
            'print(a + b);\n'
    },
    "if_else": {
        "label": "If / else with logical condition",
        "source":
            'int a = 7;\n'
            'int b = 12;\n'
            'if ((a < b) & (a != 0)) {\n'
            '    print(a + b);\n'
            '} else {\n'
            '    print(0);\n'
            '}\n'
    },
    "while_loop": {
        "label": "While loop",
        "source":
            'int i = 0;\n'
            'while (i < 5) {\n'
            '    print(i);\n'
            '    i = i + 1;\n'
            '}\n'
    },
    "for_loop": {
        "label": "For loop (custom from/to/step syntax)",
        "source":
            'int k;\n'
            'k = 0;\n'
            'for k from (0) to (5) step (1) {\n'
            '    print(k);\n'
            '}\n'
    },
    "switch": {
        "label": "Switch / case / default",
        "source":
            'int x = 2;\n'
            'switch (x) {\n'
            '    case (1) {\n'
            '        print(1);\n'
            '    }\n'
            '    case (2) {\n'
            '        print(2);\n'
            '    }\n'
            '    default {\n'
            '        print(0);\n'
            '    }\n'
            '}\n'
    },
    "function": {
        "label": "Function with parameter and return",
        "source":
            'int square(int n) {\n'
            '    return n * n;\n'
            '}\n'
            '\n'
            'int r;\n'
            'r = square(4);\n'
            'print(r);\n'
    },
    "repeat_until": {
        "label": "Repeat-until (test at bottom)",
        "source":
            'int j = 0;\n'
            'repeat {\n'
            '    j = j + 1;\n'
            '    print(j);\n'
            '} until (j == 3)\n'
    },
    "scopes": {
        "label": "Nested scopes & shadowing",
        "source":
            'int a = 5;\n'
            '{\n'
            '    int a = 99;\n'
            '    print(a);\n'
            '}\n'
            'print(a);\n'
    },
    "err_redec": {
        "label": "Error: redeclaration",
        "source":
            'int a = 5;\n'
            'int a = 7;\n'
    },
    "err_uninit": {
        "label": "Error: use before init",
        "source":
            'int a;\n'
            'int b = a + 1;\n'
    },
    "err_type": {
        "label": "Error: type mismatch",
        "source":
            'int x = "hello";\n'
    },
    "err_unused": {
        "label": "Warning: unused variable",
        "source":
            'int a = 5;\n'
            'int b = 10;\n'
            'print(a);\n'
            '   // b is never read - watch the warning panel\n'
    },
}


def _read_text(path: Path) -> str:
    """Read a file's contents as UTF-8, return '' if missing."""
    try:
        return path.read_text(encoding="utf-8")
    except FileNotFoundError:
        return ""
    except Exception as exc:
        return f"<<error reading {path.name}: {exc}>>"


def _purge_outputs():
    """Delete the four output files before each compile run so stale
    content from a previous run never leaks into the response."""
    for name in (
        "Quadruples.out",
        "SymbolTable.out",
        "SymbolTableVisualiser.out",
        "SemanticAnalysis.out",
    ):
        try:
            (HERE / name).unlink()
        except FileNotFoundError:
            pass


def _compile(source_text: str) -> dict:
    """Drive Compiler.exe over a temp src file and gather all outputs."""
    if not COMPILER.exists():
        return {
            "ok": False,
            "error": "Compiler.exe not found - rebuild via make first.",
        }

    # Write source to a UNIQUE temp file so re-entrancy doesn't race.
    # Using a NamedTemporaryFile inside the project dir so the compiler can
    # open it the same way it opens src.txt.
    tmp = tempfile.NamedTemporaryFile(
        mode="w",
        suffix=".txt",
        prefix="play_",
        dir=str(HERE),
        delete=False,
        encoding="utf-8",
    )
    tmp.write(source_text)
    tmp.close()
    tmp_path = Path(tmp.name)

    _purge_outputs()

    try:
        result = subprocess.run(
            [str(COMPILER), str(tmp_path.name)],
            capture_output=True,
            text=True,
            cwd=str(HERE),
            timeout=10,
            encoding="utf-8",
            errors="replace",
        )
    except subprocess.TimeoutExpired:
        return {
            "ok": False,
            "error": "Compiler timed out after 10 seconds.",
        }
    finally:
        try:
            tmp_path.unlink()
        except FileNotFoundError:
            pass

    return {
        "ok": True,
        "exitCode": result.returncode,
        "stderr": result.stderr or "",
        "stdout": result.stdout or "",
        "quadruples":            _read_text(HERE / "Quadruples.out"),
        "symbolTableVisualiser": _read_text(HERE / "SymbolTableVisualiser.out"),
        "semanticAnalysis":      _read_text(HERE / "SemanticAnalysis.out"),
        "symbolTableLog":        _read_text(HERE / "SymbolTable.out"),
    }


class Handler(BaseHTTPRequestHandler):
    # Quiet the request log so the terminal stays readable while typing.
    def log_message(self, fmt, *args):
        return

    def _send_json(self, status, payload):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            if not HTML_FILE.exists():
                self.send_error(500, "playground.html missing")
                return
            content = HTML_FILE.read_bytes()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(content)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(content)
            return

        if self.path == "/examples":
            self._send_json(200, EXAMPLES)
            return

        self.send_error(404)

    def do_POST(self):
        if self.path != "/compile":
            self.send_error(404)
            return

        try:
            length = int(self.headers.get("Content-Length", "0"))
            raw = self.rfile.read(length) if length > 0 else b""
            payload = json.loads(raw.decode("utf-8")) if raw else {}
            source = payload.get("source", "")
        except Exception as exc:
            self._send_json(400, {"ok": False, "error": f"bad request: {exc}"})
            return

        result = _compile(source)
        self._send_json(200, result)


def main():
    if not COMPILER.exists():
        print(f"WARNING: {COMPILER} not found. Run `make` (or the bison/flex/gcc")
        print("commands manually) before starting the playground.")

    server = ThreadingHTTPServer(("localhost", PORT), Handler)
    url = f"http://localhost:{PORT}"
    print(f"Compiler Playground running at {url}")
    print("Press Ctrl+C to stop.")

    # Best-effort: open the browser on first start.
    try:
        webbrowser.open(url)
    except Exception:
        pass

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")


if __name__ == "__main__":
    main()
