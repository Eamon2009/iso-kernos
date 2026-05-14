# Architecture

`iso-kernos` is split into four layers:

- `core`: platform-neutral metric types, the `IMonitor` interface, monitor factory, config parsing, and aggregation.
- `platform`: native OS implementations. Linux reads `/proc` and `/sys`; Windows uses Win32 and DXGI; macOS currently exposes a buildable stub.
- `display`: terminal rendering and formatting strategies for dashboard, compact, detailed, and JSON output.
- `alerts`: threshold evaluation and JSON-line logging.

The application entry point creates a platform monitor through `MonitorFactory`, collects a `SystemMetrics` snapshot through `MetricCollector`, renders it through `TerminalUI`, evaluates alerts, and optionally appends JSON logs.
