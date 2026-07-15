#!/usr/bin/env python3
import os
import re

def minify_html(html, html_dir):
    # 1. Resolve and inject <link rel="stylesheet" href="style.css">
    style_path = os.path.join(html_dir, "style.css")
    if os.path.exists(style_path):
        with open(style_path, "r", encoding="utf-8") as f:
            css_raw = f.read()
        # Simple CSS minifier
        css_min = re.sub(r'/\*.*?\*/', '', css_raw, flags=re.DOTALL)
        css_min = re.sub(r'\s+', ' ', css_min)
        css_min = re.sub(r'\s*([\{\};:,])\s*', r'\1', css_min)
        
        # Replace the link tag with the inline style block
        link_pattern = re.compile(r'<link\s+rel=["\']stylesheet["\']\s+href=["\']style\.css["\']\s*/?>', re.IGNORECASE)
        html = link_pattern.sub(f'<style>{css_min.strip()}</style>', html)

    # 2. Remove HTML comments
    html = re.sub(r'<!--.*?-->', '', html, flags=re.DOTALL)
    
    scripts = []
    styles = []
    
    # 3. Minify inline CSS and save to placeholder
    def css_replacer(match):
        css = match.group(1)
        css = re.sub(r'/\*.*?\*/', '', css, flags=re.DOTALL)
        css = re.sub(r'\s+', ' ', css)
        css = re.sub(r'\s*([\{\};:,])\s*', r'\1', css)
        placeholder = f"___STYLE_PLACEHOLDER_{len(styles)}___"
        styles.append(f'<style>{css.strip()}</style>')
        return placeholder
    
    html = re.compile(r'<style>(.*?)</style>', re.DOTALL | re.IGNORECASE).sub(css_replacer, html)
    
    # 4. Minify inline JS and save to placeholder
    def js_replacer(match):
        js = match.group(1)
        lines = []
        for line in js.splitlines():
            stripped = line.strip()
            # Skip full comment lines
            if stripped.startswith('//'):
                continue
            # Basic comment removal (avoiding stripping URLs)
            if ' //' in stripped and 'http://' not in stripped and 'https://' not in stripped:
                parts = stripped.split(' //', 1)
                stripped = parts[0].strip()
            if stripped:
                lines.append(stripped)
        
        # We join with newlines to ensure semicolons are not bypassed
        js = '\n'.join(lines)
        js = re.sub(r'/\*.*?\*/', '', js, flags=re.DOTALL)
        js = re.sub(r'[ \t]+', ' ', js)
        placeholder = f"___SCRIPT_PLACEHOLDER_{len(scripts)}___"
        scripts.append(f'<script>{js.strip()}</script>')
        return placeholder
        
    html = re.compile(r'<script>(.*?)</script>', re.DOTALL | re.IGNORECASE).sub(js_replacer, html)
    
    # 5. Remove multiple spaces and newlines outside tag contents
    html = re.sub(r'>\s+<', '><', html)
    html = re.sub(r'\s+', ' ', html)
    
    # 6. Restore styles and scripts
    for i, style_content in enumerate(styles):
        html = html.replace(f"___STYLE_PLACEHOLDER_{i}___", style_content)
    for i, script_content in enumerate(scripts):
        html = html.replace(f"___SCRIPT_PLACEHOLDER_{i}___", script_content)
        
    return html.strip()

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    html_dir = os.path.join(script_dir, "html")
    
    # 1. Generate index_html.h
    index_html_path = os.path.join(html_dir, "index.html")
    index_h_path = os.path.join(script_dir, "index_html.h")
    
    if os.path.exists(index_html_path):
        with open(index_html_path, "r", encoding="utf-8") as f:
            index_content = f.read()
            
        original_size = len(index_content)
        minified_content = minify_html(index_content, html_dir)
        minified_size = len(minified_content)
        savings = original_size - minified_size
        
        with open(index_h_path, "w", encoding="utf-8") as f_out:
            f_out.write("#ifndef INDEX_HTML_H\n")
            f_out.write("#define INDEX_HTML_H\n\n")
            f_out.write("#include <pgmspace.h>\n\n")
            f_out.write("const char INDEX_HTML[] PROGMEM = R\"rawliteral(\n")
            f_out.write(minified_content)
            f_out.write("\n)rawliteral\";\n\n")
            f_out.write("#endif // INDEX_HTML_H\n")
        print(f"Generated index_html.h (Minified: {original_size} -> {minified_size} bytes, saved {savings} bytes / {savings/original_size*100:.1f}%)")
    else:
        print("WARNING: html/index.html not found!")

    # 1b. Generate admin_html.h
    admin_html_path = os.path.join(html_dir, "admin.html")
    admin_h_path = os.path.join(script_dir, "admin_html.h")
    
    if os.path.exists(admin_html_path):
        with open(admin_html_path, "r", encoding="utf-8") as f:
            admin_content = f.read()
            
        original_size = len(admin_content)
        minified_content = minify_html(admin_content, html_dir)
        minified_size = len(minified_content)
        savings = original_size - minified_size
        
        with open(admin_h_path, "w", encoding="utf-8") as f_out:
            f_out.write("#ifndef ADMIN_HTML_H\n")
            f_out.write("#define ADMIN_HTML_H\n\n")
            f_out.write("#include <pgmspace.h>\n\n")
            f_out.write("const char ADMIN_HTML[] PROGMEM = R\"rawliteral(\n")
            f_out.write(minified_content)
            f_out.write("\n)rawliteral\";\n\n")
            f_out.write("#endif // ADMIN_HTML_H\n")
        print(f"Generated admin_html.h (Minified: {original_size} -> {minified_size} bytes, saved {savings} bytes / {savings/original_size*100:.1f}%)")
    else:
        print("WARNING: html/admin.html not found!")

    # 2. Generate update_html.h
    update_html_path = os.path.join(html_dir, "update.html")
    update_h_path = os.path.join(script_dir, "update_html.h")
    
    # Read and minify file
    up_content = ""
    if os.path.exists(update_html_path):
        with open(update_html_path, "r", encoding="utf-8") as f:
            up_content = minify_html(f.read(), html_dir)
    else:
        print("WARNING: html/update.html not found!")
        
    with open(update_h_path, "w", encoding="utf-8") as f_out:
        f_out.write("#ifndef UPDATE_HTML_H\n")
        f_out.write("#define UPDATE_HTML_H\n\n")
        f_out.write("#include <pgmspace.h>\n\n")
        
        f_out.write("const char UPDATE_HTML[] PROGMEM = R\"rawliteral(\n")
        f_out.write(up_content)
        f_out.write("\n)rawliteral\";\n\n")
        
        f_out.write("const char UPDATE_ERR_HTML[] PROGMEM = R\"rawliteral(\n")
        f_out.write("FAIL")
        f_out.write("\n)rawliteral\";\n\n")
        
        f_out.write("const char UPDATE_OK_HTML[] PROGMEM = R\"rawliteral(\n")
        f_out.write("OK")
        f_out.write("\n)rawliteral\";\n\n")
        
        f_out.write("#endif // UPDATE_HTML_H\n")
    print("Generated update_html.h (bundled and minified)")

if __name__ == "__main__":
    main()
