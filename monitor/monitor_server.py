#!/usr/bin/env python3
"""
Priority Inheritance Monitor - Bridge Server
Reads xv6 kernel output, parses JSON events, and serves data to web dashboard
"""

from flask import Flask, render_template, jsonify
from flask_cors import CORS
from flask_socketio import SocketIO, emit
import json
import subprocess
import threading
import time
from collections import defaultdict, deque
from datetime import datetime
import re

app = Flask(__name__)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Global data store
class MonitorData:
    def __init__(self):
        self.events = deque(maxlen=1000)  # Last 1000 events
        self.priority_boosts = []  # All boost events
        self.inversions_detected = []  # Priority inversions
        self.process_stats = defaultdict(lambda: {
            'pid': 0,
            'name': '',
            'priority': 0,
            'boosts_received': 0,
            'boosts_given': 0,
            'locks_held': 0,
            'blocks': 0,
            'total_boost_time': 0
        })
        self.total_boosts = 0
        self.total_inversions = 0
        self.start_time = datetime.now()
        self.active_processes = {}
        
monitor = MonitorData()

def parse_json_event(line):
    """Extract JSON from kernel output line"""
    try:
        # Find JSON object in line
        match = re.search(r'\{.*\}', line)
        if match:
            return json.loads(match.group())
    except:
        pass
    return None

def process_event(event):
    """Process parsed event and update statistics"""
    timestamp = datetime.now().isoformat()
    event['timestamp'] = timestamp
    
    monitor.events.append(event)
    
    if event['event'] == 'priority_boost':
        monitor.total_boosts += 1
        holder_pid = event['holder_pid']
        waiter_pid = event['waiter_pid']
        
        monitor.process_stats[holder_pid]['boosts_received'] += 1
        monitor.process_stats[waiter_pid]['boosts_given'] += 1
        
        boost_event = {
            'timestamp': timestamp,
            'holder_pid': holder_pid,
            'waiter_pid': waiter_pid,
            'old_priority': event['old_priority'],
            'new_priority': event['new_priority']
        }
        monitor.priority_boosts.append(boost_event)
        
        # Emit to connected clients
        socketio.emit('priority_boost', boost_event)
        
    elif event['event'] == 'lock_request':
        pid = event['pid']
        monitor.process_stats[pid]['blocks'] += 1
        
        # Detect potential inversion
        if event['holder_priority'] > event['priority']:
            inversion = {
                'timestamp': timestamp,
                'high_priority_pid': pid,
                'high_priority': event['priority'],
                'low_priority_pid': event['holder_pid'],
                'low_priority': event['holder_priority']
            }
            monitor.inversions_detected.append(inversion)
            monitor.total_inversions += 1
            socketio.emit('inversion_detected', inversion)
            
    elif event['event'] == 'lock_acquired':
        pid = event['pid']
        monitor.process_stats[pid]['locks_held'] += 1
        monitor.active_processes[pid] = {
            'priority': event['priority'],
            'state': 'running'
        }
        
    elif event['event'] == 'lock_released':
        pid = event['pid']
        if pid in monitor.active_processes:
            monitor.active_processes[pid]['state'] = 'idle'
    
    # Emit general update
    socketio.emit('stats_update', get_stats())

def monitor_xv6_output(logfile_path):
    """Monitor xv6 output file for JSON events"""
    print(f"Monitoring {logfile_path} for priority inheritance events...")
    
    try:
        with open(logfile_path, 'r') as f:
            # Go to end of file
            f.seek(0, 2)
            
            while True:
                line = f.readline()
                if line:
                    # Check for JSON event
                    event = parse_json_event(line)
                    if event:
                        process_event(event)
                        print(f"Event: {event}")
                else:
                    time.sleep(0.1)
    except FileNotFoundError:
        print(f"Log file {logfile_path} not found. Waiting...")
        time.sleep(2)
        monitor_xv6_output(logfile_path)
    except Exception as e:
        print(f"Error monitoring file: {e}")
        time.sleep(2)
        monitor_xv6_output(logfile_path)

def get_stats():
    """Get current statistics"""
    uptime = (datetime.now() - monitor.start_time).total_seconds()
    
    return {
        'total_boosts': monitor.total_boosts,
        'total_inversions': monitor.total_inversions,
        'total_events': len(monitor.events),
        'uptime': uptime,
        'active_processes': len(monitor.active_processes),
        'process_stats': dict(monitor.process_stats),
        'recent_events': list(monitor.events)[-20:],  # Last 20 events
        'boost_rate': monitor.total_boosts / uptime if uptime > 0 else 0
    }

@app.route('/')
def index():
    """Serve dashboard"""
    return render_template('dashboard.html')

@app.route('/api/stats')
def api_stats():
    """Get current statistics"""
    return jsonify(get_stats())

@app.route('/api/boosts')
def api_boosts():
    """Get all priority boost events"""
    return jsonify({
        'boosts': monitor.priority_boosts[-100:],  # Last 100
        'total': len(monitor.priority_boosts)
    })

@app.route('/api/inversions')
def api_inversions():
    """Get detected priority inversions"""
    return jsonify({
        'inversions': monitor.inversions_detected[-100:],
        'total': len(monitor.inversions_detected)
    })

@app.route('/api/processes')
def api_processes():
    """Get process statistics"""
    return jsonify({
        'processes': dict(monitor.process_stats),
        'active': monitor.active_processes
    })

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    print('Client connected')
    emit('stats_update', get_stats())

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    print('Client disconnected')

if __name__ == '__main__':
    import sys
    
    # Start monitoring thread
    logfile = sys.argv[1] if len(sys.argv) > 1 else 'xv6_output.log'
    monitor_thread = threading.Thread(target=monitor_xv6_output, args=(logfile,), daemon=True)
    monitor_thread.start()
    
    print("=" * 60)
    print("Priority Inheritance Monitor Server")
    print("=" * 60)
    print(f"Monitoring: {logfile}")
    print("Dashboard: http://localhost:5000")
    print("=" * 60)
    
    # Start Flask server
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, use_reloader=False)
