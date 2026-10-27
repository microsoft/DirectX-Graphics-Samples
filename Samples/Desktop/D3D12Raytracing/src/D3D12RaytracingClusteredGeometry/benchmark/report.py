"""Offline report rendering shared by the sweep and comparison commands."""

from __future__ import annotations

import html
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


@dataclass
class Chart:
    title: str
    category: str
    description: str
    x_label: str
    unit: str
    series: dict[str, list[tuple[Any, float | None]]] = field(default_factory=dict)
    chart_type: str = "line"
    numeric_x: bool = False
    stacked: bool = False
    color_groups: dict[str, str] = field(default_factory=dict)
    stacks: dict[str, str] = field(default_factory=dict)
    x_tooltips: dict[str, str] = field(default_factory=dict)
    line_dashes: dict[str, list[int]] = field(default_factory=dict)
    point_styles: dict[str, str] = field(default_factory=dict)


def script_json(value: Any) -> str:
    """Keep data, including literal HTML closing tags, inside its script."""
    return json.dumps(value, allow_nan=False).replace("<", "\\u003c").replace(
        ">", "\\u003e").replace("&", "\\u0026")


THEME_SCRIPT = """<script>
  (() => {
    const param = new URLSearchParams(window.location.search).get("scoutTheme");
    const theme =
      param || (window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light");
    document.documentElement.setAttribute("data-theme", theme);
  })();
</script>"""

STYLE = """
:root {
  color-scheme: light;
  --cp-bg: #f7f4ef;
  --cp-bg-elevated: #fcfbf8;
  --cp-surface: #ffffff;
  --cp-surface-soft: #f5f5f5;
  --cp-border: #dedede;
  --cp-border-strong: #919191;
  --cp-text: #242424;
  --cp-text-muted: #5c5c5c;
  --cp-text-soft: #6f6f6f;
  --cp-accent: #b11f4b;
  --cp-accent-hover: #9a1a41;
  --cp-accent-soft: rgba(177, 31, 75, 0.08);
  --cp-accent-fg: #ffffff;
  --cp-success: #16a34a;
  --cp-danger: #dc2626;
  --cp-warning: #f59e0b;
  --cp-link: #0078d4;
  --cp-shadow: 0 18px 48px rgba(0, 0, 0, 0.12);
  --cp-overlay: rgba(255, 255, 255, 0.8);
  --cp-panel: rgba(255, 255, 255, 0.86);
  --cp-panel-strong: rgba(255, 255, 255, 0.96);
  --cp-sheen: rgba(255, 255, 255, 0.55);
  --cp-highlight: rgba(177, 31, 75, 0.12);
}
html[data-theme="dark"] {
  color-scheme: dark;
  --cp-bg: #3d3b3a;
  --cp-bg-elevated: #343231;
  --cp-surface: #292929;
  --cp-surface-soft: #2e2e2e;
  --cp-border: #474747;
  --cp-border-strong: #5f5f5f;
  --cp-text: #dedede;
  --cp-text-muted: #919191;
  --cp-text-soft: #b0b0b0;
  --cp-accent: #fd8ea1;
  --cp-accent-hover: #fb7b91;
  --cp-accent-soft: rgba(253, 142, 161, 0.14);
  --cp-accent-fg: #1a1a1a;
  --cp-success: #4ade80;
  --cp-danger: #f87171;
  --cp-warning: #fbbf24;
  --cp-link: #4da6ff;
  --cp-shadow: 0 18px 48px rgba(0, 0, 0, 0.32);
  --cp-overlay: rgba(41, 41, 41, 0.88);
  --cp-panel: rgba(41, 41, 41, 0.72);
  --cp-panel-strong: rgba(41, 41, 41, 0.96);
  --cp-sheen: rgba(255, 255, 255, 0.04);
  --cp-highlight: rgba(253, 142, 161, 0.12);
}
* { box-sizing: border-box; }
body {
  margin: 0 auto; padding: 24px; max-width: 1440px;
  background: var(--cp-bg); color: var(--cp-text);
  font-family: "Segoe UI", Aptos, Calibri, -apple-system, BlinkMacSystemFont, sans-serif;
}
h1 { font-size: 26px; margin: 0 0 8px; }
h2 { font-size: 20px; margin: 32px 0 12px; color: var(--cp-accent); }
h3 { font-size: 16px; margin: 0 0 8px; }
p, li { line-height: 1.5; }
.subtitle, .desc { color: var(--cp-text-muted); font-size: 13px; }
.grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(min(100%, 520px), 1fr)); gap: 16px; }
.chart-wide { grid-column: 1 / -1; }
.card {
  background: var(--cp-surface); border: 1px solid var(--cp-border);
  border-radius: 16px; padding: 16px; margin: 12px 0;
}
.plot { position: relative; height: 320px; }
.scroll { overflow-x: auto; }
table { border-collapse: collapse; width: 100%; font-size: 12px; }
th, td { padding: 8px; border-bottom: 1px solid var(--cp-border); text-align: left; vertical-align: top; }
th { color: var(--cp-text-muted); font-weight: 600; }
td { overflow-wrap: anywhere; }
code { font-family: Consolas, "Courier New", Courier, monospace; }
.study-index { display: flex; flex-wrap: wrap; gap: 8px; }
.study-index a {
  color: var(--cp-link); background: var(--cp-surface-soft);
  border: 1px solid var(--cp-border); border-radius: 0.625rem;
  padding: 8px 12px; text-decoration: none; font-size: 13px;
}
.study-index a:hover { background: var(--cp-highlight); }
"""

CHART_SCRIPT = """
const css = name => getComputedStyle(document.documentElement).getPropertyValue(name).trim();
Chart.defaults.color = css('--cp-text');
Chart.defaults.borderColor = css('--cp-border');
Chart.defaults.font.family = '"Segoe UI", Aptos, Calibri, sans-serif';
const palette = ['--cp-accent', '--cp-link', '--cp-success', '--cp-warning',
                 '--cp-text-soft', '--cp-danger'];
const colorGroups = [...new Set(reportCharts.flatMap(c => c.series.map(s => s.color_group)))];
const format = (value, unit) => {
  if (value === null) return 'not measured';
  if (unit === 'bytes') {
    const sizes = ['B', 'KiB', 'MiB', 'GiB'];
    let i = 0;
    while (value >= 1024 && i < sizes.length - 1) { value /= 1024; ++i; }
    return value.toLocaleString(undefined, {maximumFractionDigits: 2}) + ' ' + sizes[i];
  }
  return value.toLocaleString(undefined, {maximumSignificantDigits: 6}) + ' ' + unit;
};
for (const [index, spec] of reportCharts.entries()) {
  const labels = [...new Set(spec.series.flatMap(series => series.points.map(p => p[0])))];
  if (spec.numeric_x) labels.sort((a, b) => a - b);
  const datasets = spec.series.map((series, i) => {
    const values = new Map(series.points);
    const color = css(palette[colorGroups.indexOf(series.color_group) % palette.length]);
    return {
      label: series.label,
      data: labels.map(x => ({x, y: values.has(x) ? values.get(x) : null})),
      borderColor: color, backgroundColor: color, borderWidth: 2,
      pointRadius: spec.chart_type === 'line' ? 3 : 0,
      pointStyle: series.point_style,
      tension: 0, spanGaps: false, stack: spec.stacked ? series.stack : undefined,
      borderDash: series.line_dash ?? (series.label.includes('traditional refit') ? [2, 3] :
                  series.label.includes('traditional rebuild') ? [6, 3] : []
                  )
    };
  });
  new Chart(document.getElementById('chart-' + index), {
    type: spec.chart_type, data: {labels, datasets},
    plugins: [{
      id: 'report-legend-space',
      afterLayout(chart) {
        const plot = chart.canvas.parentElement;
        const height = Math.max(320, Math.ceil(chart.legend.height + 280),
                                Math.ceil(chart.height - chart.chartArea.height + 240));
        if (plot.style.height !== height + 'px') {
          plot.style.height = height + 'px';
          requestAnimationFrame(() => chart.resize());
        }
      }
    }],
    options: {
      responsive: true, maintainAspectRatio: false,
      scales: {
        x: {type: spec.numeric_x ? 'linear' : 'category', stacked: spec.stacked,
            title: {display: true, text: spec.x_label}},
        y: {type: 'linear', beginAtZero: true, stacked: spec.stacked,
            title: {display: true, text: spec.unit},
            ticks: {callback: value => format(value, spec.unit)}}
      },
      plugins: {
        legend: {labels: {usePointStyle: spec.chart_type === 'line'}},
        tooltip: {callbacks: {
          title: items => {
            if (!items.length) return '';
            const key = String(items[0].raw.x);
            return spec.x_tooltips[key] ? spec.x_tooltips[key].split('\\n') : key;
          },
          label: item => item.dataset.label + ': ' + format(item.raw.y, spec.unit)
        }}
      }
    }
  });
}
"""


def render_report(title: str, subtitle: str, charts: list[Chart],
                  tables: list[tuple[str, list[str], list[list[Any]]]],
                  notes: list[str],
                  overview_tables: list[tuple[str, list[str], list[list[Any]]]] | None = None) -> str:
    """Embed Chart.js locally; never silently depend on a remote CDN."""
    library = Path(__file__).with_name("chart.umd.min.js").read_text(encoding="utf-8")
    license_text = Path(__file__).with_name("LICENSE.Chart.js.txt").read_text(encoding="utf-8")
    escape = lambda value: html.escape(str(value), quote=True)
    def render_table(heading: str, columns: list[str], rows: list[list[Any]]) -> str:
        return (f"<section class='card'><h2>{escape(heading)}</h2><div class='scroll'>"
                "<table><thead><tr>" +
                "".join(f"<th>{escape(column)}</th>" for column in columns) +
                "</tr></thead><tbody>" +
                "".join("<tr>" + "".join(f"<td>{escape(cell)}</td>" for cell in row) +
                        "</tr>" for row in rows) + "</tbody></table></div></section>")
    body = [f"<h1>{escape(title)}</h1><p class='subtitle'>{escape(subtitle)}</p>"]
    if notes:
        body.append("<section class='card'><h2>Reading this report</h2><ul>" +
                    "".join(f"<li>{escape(note)}</li>" for note in notes) + "</ul></section>")
    for heading, columns, rows in overview_tables or []:
        body.append(render_table(heading, columns, rows))
    categories = list(dict.fromkeys(chart.category for chart in charts))
    if categories:
        body.append("<nav class='card' aria-label='Benchmark studies'><h2>Studies</h2>"
                    "<div class='study-index'>" + "".join(
                        f"<a href='#study-{index}'>{escape(category)}</a>"
                        for index, category in enumerate(categories)) + "</div></nav>")
    current_category = None
    for index, chart in enumerate(charts):
        if chart.category != current_category:
            if current_category is not None:
                body.append("</div>")
            body.append(f"<h2 id='study-{categories.index(chart.category)}'>"
                        f"{escape(chart.category)}</h2><div class='grid'>")
            current_category = chart.category
        wide = " chart-wide" if len(chart.series) > 3 else ""
        body.append(f"<section class='card{wide}'><h3>{escape(chart.title)}</h3>"
                    f"<p class='desc'>{escape(chart.description)}</p>"
                    f"<div class='plot'><canvas id='chart-{index}'></canvas></div></section>")
    if current_category is not None:
        body.append("</div>")
    for heading, columns, rows in tables:
        body.append(render_table(heading, columns, rows))
    specs = [
        dict(title=c.title, unit=c.unit, x_label=c.x_label, chart_type=c.chart_type,
             numeric_x=c.numeric_x, stacked=c.stacked, x_tooltips=c.x_tooltips,
             series=[dict(label=label, points=points, color_group=c.color_groups.get(label, label),
                          stack=c.stacks.get(label, "all"),
                          line_dash=c.line_dashes.get(label),
                          point_style=c.point_styles.get(label, "circle"))
                     for label, points in c.series.items()])
        for c in charts
    ]
    return ("<!doctype html><html lang='en'><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
            f"<title>{escape(title)}</title>{THEME_SCRIPT}<style>{STYLE}</style>"
            f"<!-- {escape(license_text)} --><script>{library}</script></head><body>{''.join(body)}"
            f"<script>const reportCharts = {script_json(specs)};\n{CHART_SCRIPT}</script>"
            "</body></html>")
