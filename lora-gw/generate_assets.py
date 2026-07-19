#!/usr/bin/env python3
import os
import re


def minify_css(css):
    """Full-pass CSS minifier."""
    # 1. Remove block comments
    css = re.sub(r'/\*.*?\*/', '', css, flags=re.DOTALL)
    # 2. Collapse all whitespace (newlines, tabs, multiple spaces) to one space
    css = re.sub(r'\s+', ' ', css)
    # 3. Remove spaces around structural characters
    css = re.sub(r'\s*([{}:;,>~+])\s*', r'\1', css)
    # 4. Remove trailing semicolon before closing brace (saves ~1 byte/rule)
    css = re.sub(r';}', '}', css)
    # 5. Remove leading zeros from decimals: 0.5 -> .5
    css = re.sub(r'(?<![0-9])0\.([0-9])', r'.\1', css)
    # 6. Remove units from explicit zero values: 0px -> 0, 0em -> 0, etc.
    css = re.sub(r'(?<![0-9])0(px|em|rem|vw|vh|pt|cm|mm|in|ch|ex)', r'0', css)
    # 7. Lowercase hex color codes
    css = re.sub(r'#[0-9a-fA-F]{3,8}', lambda m: m.group().lower(), css)
    # 8. Shorten 6-char hex to 3-char where pairs match (#aabbcc -> #abc)
    css = re.sub(
        r'#([0-9a-f])\1([0-9a-f])\2([0-9a-f])\3(?![0-9a-f])',
        lambda m: f'#{m.group(1)}{m.group(2)}{m.group(3)}',
        css
    )
    return css.strip()


def minify_js(js):
    """Basic JS minifier: strips comments and collapses whitespace."""
    lines = []
    for line in js.splitlines():
        stripped = line.strip()
        if stripped.startswith('//'):
            continue
        # Strip inline comments but avoid stripping URLs
        if ' //' in stripped and 'http://' not in stripped and 'https://' not in stripped:
            stripped = stripped.split(' //', 1)[0].strip()
        if stripped:
            lines.append(stripped)
    js = '\n'.join(lines)
    js = re.sub(r'/\*.*?\*/', '', js, flags=re.DOTALL)
    js = re.sub(r'[ \t]+', ' ', js)
    return js.strip()


def minify_html(html, html_dir):
    # 1. Resolve and inline <link rel="stylesheet" href="style.css">
    style_path = os.path.join(html_dir, "style.css")
    if os.path.exists(style_path):
        with open(style_path, "r", encoding="utf-8") as f:
            css_raw = f.read()
        css_min = minify_css(css_raw)
        css_original_size = len(css_raw)
        css_minified_size = len(css_min)
        link_pattern = re.compile(
            r'<link\s+rel=["\']stylesheet["\']\s+href=["\']style\.css["\']\s*/?>', re.IGNORECASE
        )
        html = link_pattern.sub(f'<style>{css_min}</style>', html)
        print(f"  CSS inlined: {css_original_size} -> {css_minified_size} bytes "
              f"({(1 - css_minified_size/css_original_size)*100:.1f}% reduction)")

    # 2. Remove HTML comments
    html = re.sub(r'<!--.*?-->', '', html, flags=re.DOTALL)

    scripts = []
    styles = []

    # 3. Minify and stash inline <style> blocks
    def css_replacer(match):
        minified = minify_css(match.group(1))
        placeholder = f"___STYLE_PLACEHOLDER_{len(styles)}___"
        styles.append(f'<style>{minified}</style>')
        return placeholder
    html = re.compile(r'<style>(.*?)</style>', re.DOTALL | re.IGNORECASE).sub(css_replacer, html)

    # 4. Minify and stash inline <script> blocks
    def js_replacer(match):
        minified = minify_js(match.group(1))
        placeholder = f"___SCRIPT_PLACEHOLDER_{len(scripts)}___"
        scripts.append(f'<script>{minified}</script>')
        return placeholder
    html = re.compile(r'<script>(.*?)</script>', re.DOTALL | re.IGNORECASE).sub(js_replacer, html)

    # 5. Collapse whitespace between HTML tags
    html = re.sub(r'>\s+<', '><', html)
    html = re.sub(r'\s+', ' ', html)

    # 6. Restore stashed styles and scripts
    for i, style_content in enumerate(styles):
        html = html.replace(f"___STYLE_PLACEHOLDER_{i}___", style_content)
    for i, script_content in enumerate(scripts):
        html = html.replace(f"___SCRIPT_PLACEHOLDER_{i}___", script_content)

    return html.strip()


def process_html(src_path, dst_path, html_dir, guard_name, var_name):
    if not os.path.exists(src_path):
        print(f"WARNING: {src_path} not found!")
        return
    with open(src_path, "r", encoding="utf-8") as f:
        content = f.read()
    original_size = len(content)
    print(f"\n[{os.path.basename(src_path)}] HTML source: {original_size} bytes")
    minified = minify_html(content, html_dir)
    final_size = len(minified)
    net = final_size - original_size
    sign = '+' if net >= 0 else ''
    print(f"  Final bundle: {final_size} bytes (HTML+CSS+JS, {sign}{net} vs HTML-only source)")
    with open(dst_path, "w", encoding="utf-8") as f:
        f.write(f"#ifndef {guard_name}\n")
        f.write(f"#define {guard_name}\n\n")
        f.write("#include <pgmspace.h>\n\n")
        f.write(f"const char {var_name}[] PROGMEM = R\"rawliteral(\n")
        f.write(minified)
        f.write("\n)rawliteral\";\n\n")
        f.write(f"#endif // {guard_name}\n")
    print(f"  -> {os.path.basename(dst_path)} written")


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    html_dir = os.path.join(script_dir, "html")

    process_html(
        os.path.join(html_dir, "index.html"),
        os.path.join(script_dir, "index_html.h"),
        html_dir, "INDEX_HTML_H", "INDEX_HTML"
    )
    process_html(
        os.path.join(html_dir, "admin.html"),
        os.path.join(script_dir, "admin_html.h"),
        html_dir, "ADMIN_HTML_H", "ADMIN_HTML"
    )

    # update.html: bundled with OK/FAIL inline responses
    update_html_path = os.path.join(html_dir, "update.html")
    update_h_path = os.path.join(script_dir, "update_html.h")
    up_content = ""
    if os.path.exists(update_html_path):
        with open(update_html_path, "r", encoding="utf-8") as f:
            raw = f.read()
        up_content = minify_html(raw, html_dir)
        print(f"\n[update.html] -> {len(up_content)} bytes")
    else:
        print("WARNING: html/update.html not found!")

    with open(update_h_path, "w", encoding="utf-8") as f:
        f.write("#ifndef UPDATE_HTML_H\n#define UPDATE_HTML_H\n\n")
        f.write("#include <pgmspace.h>\n\n")
        f.write("const char UPDATE_HTML[] PROGMEM = R\"rawliteral(\n")
        f.write(up_content)
        f.write("\n)rawliteral\";\n\n")
        f.write("const char UPDATE_ERR_HTML[] PROGMEM = R\"rawliteral(\nFAIL\n)rawliteral\";\n\n")
        f.write("const char UPDATE_OK_HTML[] PROGMEM = R\"rawliteral(\nOK\n)rawliteral\";\n\n")
        f.write("#endif // UPDATE_HTML_H\n")
    print(f"  -> update_html.h written")


if __name__ == "__main__":
    main()
