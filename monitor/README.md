# xv6 Priority Inheritance Real-Time Monitor

A comprehensive monitoring system that tracks priority inheritance events in your xv6 kernel in real-time and displays analytics on a web dashboard.

## Features

🔍 **Real-time Monitoring**
- Live tracking of priority inheritance events
- WebSocket-based instant updates
- No page refresh needed

📊 **Analytics Dashboard**
- Total priority boosts count
- Priority inversion detection
- Process statistics and metrics
- Interactive charts and graphs

⚡ **Event Tracking**
- Lock acquisitions and releases
- Priority boosts and restorations
- Process blocking events
- Priority inversion alerts

## Architecture

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│  xv6 Kernel │─────▶│ Monitor      │◀────▶│ Web         │
│  (proc.c)   │ JSON │ Server       │ WS   │ Dashboard   │
│             │ Logs │ (Python)     │      │ (Browser)   │
└─────────────┘      └──────────────┘      └─────────────┘
```

## Installation

### 1. Setup Dependencies

```bash
chmod +x setup.sh
./setup.sh
```

### 2. Update xv6 Kernel

```bash
cp proc.c kernel/
make clean
make
```

### 3. Start Monitoring

**Terminal 1** - Run xv6 with logging:
```bash
make qemu 2>&1 | tee xv6_output.log
```

**Terminal 2** - Start monitor server:
```bash
python3 monitor_server.py xv6_output.log
```

**Browser** - Open dashboard:
```
http://localhost:5000
```

### 4. Run Tests in xv6

```bash
$ pi_detailed
$ pi_test2
```

## Dashboard Metrics

### 📈 Real-time Metrics
- **Priority Boosts**: Total number of times priority inheritance occurred
- **Inversions Detected**: Number of priority inversion cases identified
- **Active Processes**: Currently running processes
- **Uptime**: Monitor uptime

### 📊 Charts
- **Boosts Over Time**: Line graph showing boost events timeline
- **Priority Distribution**: Pie chart of process priority levels

### 📡 Events Feed
Live stream of:
- Priority boost events with PID and priority changes
- Priority inversion detections
- Lock acquisitions and releases

### 📋 Process Statistics Table
Per-process metrics:
- PID and current priority
- Boosts received (how many times boosted)
- Boosts given (how many times this process caused a boost)
- Locks held
- Times blocked

## JSON Event Format

The kernel outputs JSON events that are parsed by the monitor:

```json
{"event":"priority_boost","holder_pid":5,"old_priority":10,"new_priority":1,"waiter_pid":6,"waiter_priority":1}
{"event":"lock_acquired","pid":5,"priority":1}
{"event":"priority_restore","pid":5,"old_priority":1,"new_priority":10}
{"event":"lock_released","pid":5}
```

## API Endpoints

The monitor server provides REST APIs:

- `GET /api/stats` - Current statistics
- `GET /api/boosts` - All priority boost events
- `GET /api/inversions` - Detected priority inversions
- `GET /api/processes` - Process statistics

## WebSocket Events

Real-time events via Socket.IO:

- `stats_update` - Updated statistics
- `priority_boost` - New priority boost occurred
- `inversion_detected` - Priority inversion detected

## Troubleshooting

### Monitor not receiving events
- Check that xv6 output is being written to the log file
- Verify the log file path in monitor_server.py
- Ensure processes are actually calling test_acquire/test_release

### Dashboard not updating
- Check browser console for errors
- Verify WebSocket connection (should show "Connected")
- Ensure port 5000 is not blocked by firewall

### No JSON events in kernel output
- Verify proc.c has been updated with JSON logging
- Recompile xv6: `make clean && make`
- Check that printf is working in kernel

## Customization

### Change Monitor Port
Edit `monitor_server.py`:
```python
socketio.run(app, host='0.0.0.0', port=5000)  # Change 5000
```

### Adjust Event History
Edit `monitor_server.py`:
```python
self.events = deque(maxlen=1000)  # Change 1000
```

### Customize Dashboard
Edit `templates/dashboard.html` - modify styles, charts, or layout

## Example Output

When running `pi_detailed` in xv6, you'll see:

**Dashboard shows:**
- Priority Boosts: 3
- Inversions Detected: 1
- Active Processes: 3

**Events feed:**
```
14:23:45 - PID 5 boosted from priority 10 → 1 (waiter: PID 6)
14:23:46 - Inversion: High priority PID 6 (pri=1) blocked by low priority PID 5 (pri=10)
14:23:47 - PID 5 priority restored 1 → 10
```

## License

MIT

## Credits

Created for xv6 kernel priority inheritance monitoring and analysis.
