#!/usr/bin/env python3
"""
Enhanced Priority Inheritance Monitor
- Real-time monitoring
- File upload and analysis  
- Multiple test scenario comparison
- Advanced analytics
"""

from flask import Flask, render_template, jsonify, request, send_file
from flask_cors import CORS
from flask_socketio import SocketIO, emit
import json
import threading
import time
from collections import defaultdict, deque
from datetime import datetime
import re
import os

app = Flask(__name__)
CORS(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Data store for different test scenarios
class TestScenario:
    def __init__(self, name):
        self.name = name
        self.events = deque(maxlen=1000)
        self.priority_boosts = []
        self.inversions_detected = []
        self.process_stats = defaultdict(lambda: {
            'pid': 0,
            'name': '',
            'priority': 0,
            'initial_priority': 0,
            'boosts_received': 0,
            'boosts_given': 0,
            'locks_held': 0,
            'blocks': 0,
            'max_priority': 10,
            'min_priority': 10
        })
        self.total_boosts = 0
        self.total_inversions = 0
        self.start_time = datetime.now()
        self.end_time = None
        self.active_processes = {}
        self.timeline_events = []
        
class MonitorData:
    def __init__(self):
        self.scenarios = {
            'live': TestScenario('Live Monitor'),
            'pi_detailed': TestScenario('PI Detailed Test'),
            'pi_test2': TestScenario('PI Test 2'),
            'custom': TestScenario('Custom Analysis')
        }
        self.current_scenario = 'live'
        self.uploaded_files = []
        
monitor = MonitorData()

def parse_json_event(line):
    try:
        match = re.search(r'\{.*\}', line)
        if match:
            return json.loads(match.group())
    except:
        pass
    return None

def detect_scenario(line):
    if 'PI Detailed Test' in line or 'pi_detailed' in line:
        return 'pi_detailed'
    elif 'PI Test 2' in line or 'pi_test2' in line:
        return 'pi_test2'
    return 'live'

def process_event(event, scenario_name='live'):
    scenario = monitor.scenarios[scenario_name]
    timestamp = datetime.now().isoformat()
    event['timestamp'] = timestamp
    
    scenario.events.append(event)
    scenario.timeline_events.append({
        'time': timestamp,
        'type': event['event'],
        'data': event
    })
    
    if event['event'] == 'priority_boost':
        scenario.total_boosts += 1
        holder_pid = event['holder_pid']
        waiter_pid = event['waiter_pid']
        
        scenario.process_stats[holder_pid]['boosts_received'] += 1
        scenario.process_stats[holder_pid]['max_priority'] = min(
            scenario.process_stats[holder_pid]['max_priority'],
            event['new_priority']
        )
        scenario.process_stats[waiter_pid]['boosts_given'] += 1
        
        boost_event = {
            'timestamp': timestamp,
            'holder_pid': holder_pid,
            'waiter_pid': waiter_pid,
            'old_priority': event['old_priority'],
            'new_priority': event['new_priority']
        }
        scenario.priority_boosts.append(boost_event)
        
        socketio.emit('priority_boost', {
            'scenario': scenario_name,
            'event': boost_event
        })
        
    elif event['event'] == 'lock_request':
        pid = event['pid']
        scenario.process_stats[pid]['blocks'] += 1
        
        if event['holder_priority'] > event['priority']:
            inversion = {
                'timestamp': timestamp,
                'high_priority_pid': pid,
                'high_priority': event['priority'],
                'low_priority_pid': event['holder_pid'],
                'low_priority': event['holder_priority'],
                'severity': event['holder_priority'] - event['priority']
            }
            scenario.inversions_detected.append(inversion)
            scenario.total_inversions += 1
            socketio.emit('inversion_detected', {
                'scenario': scenario_name,
                'inversion': inversion
            })
            
    elif event['event'] == 'lock_acquired':
        pid = event['pid']
        scenario.process_stats[pid]['locks_held'] += 1
        if scenario.process_stats[pid]['initial_priority'] == 0:
            scenario.process_stats[pid]['initial_priority'] = event['priority']
        scenario.process_stats[pid]['priority'] = event['priority']
        scenario.active_processes[pid] = {'priority': event['priority'], 'state': 'running'}
        
    elif event['event'] == 'lock_released':
        pid = event['pid']
        if pid in scenario.active_processes:
            scenario.active_processes[pid]['state'] = 'idle'
            
    elif event['event'] == 'priority_restore':
        pid = event['pid']
        scenario.process_stats[pid]['priority'] = event['new_priority']
    
    socketio.emit('stats_update', {
        'scenario': scenario_name,
        'stats': get_scenario_stats(scenario_name)
    })

def analyze_log_file(filepath, scenario_name='custom'):
    scenario = monitor.scenarios[scenario_name]
    scenario.events.clear()
    scenario.priority_boosts.clear()
    scenario.inversions_detected.clear()
    scenario.process_stats.clear()
    scenario.total_boosts = 0
    scenario.total_inversions = 0
    scenario.timeline_events.clear()
    
    try:
        with open(filepath, 'r') as f:
            for line in f:
                event = parse_json_event(line)
                if event:
                    process_event(event, scenario_name)
        
        scenario.end_time = datetime.now()
        return True
    except Exception as e:
        print(f"Error: {e}")
        return False

def monitor_xv6_output(logfile_path):
    print(f"Monitoring {logfile_path}...")
    
    try:
        with open(logfile_path, 'r') as f:
            f.seek(0, 2)
            
            while True:
                line = f.readline()
                if line:
                    event = parse_json_event(line)
                    if event:
                        scenario_name = detect_scenario(line)
                        process_event(event, scenario_name)
                else:
                    time.sleep(0.1)
    except Exception as e:
        print(f"Error: {e}")
        time.sleep(2)
        monitor_xv6_output(logfile_path)

def get_scenario_stats(scenario_name):
    scenario = monitor.scenarios[scenario_name]
    uptime = (scenario.end_time or datetime.now() - scenario.start_time).total_seconds()
    
    process_list = list(scenario.process_stats.values())
    avg_boosts = sum(p['boosts_received'] for p in process_list) / len(process_list) if process_list else 0
    
    inversions_by_severity = {
        'low': sum(1 for i in scenario.inversions_detected if i['severity'] <= 3),
        'medium': sum(1 for i in scenario.inversions_detected if 3 < i['severity'] <= 6),
        'high': sum(1 for i in scenario.inversions_detected if i['severity'] > 6)
    }
    
    return {
        'name': scenario.name,
        'total_boosts': scenario.total_boosts,
        'total_inversions': scenario.total_inversions,
        'total_events': len(scenario.events),
        'uptime': uptime,
        'active_processes': len(scenario.active_processes),
        'process_stats': dict(scenario.process_stats),
        'recent_events': list(scenario.events)[-20:],
        'boost_rate': scenario.total_boosts / uptime if uptime > 0 else 0,
        'avg_boosts_per_process': avg_boosts,
        'inversions_by_severity': inversions_by_severity,
        'timeline': scenario.timeline_events[-50:]
    }

# Routes
@app.route('/')
def index():
    return render_template('dashboard.html')

@app.route('/analysis/<scenario>')
def analysis_page(scenario):
    return render_template('analysis.html', scenario=scenario)

@app.route('/compare')
def compare_page():
    return render_template('compare.html')

@app.route('/upload')
def upload_page():
    return render_template('upload.html')

@app.route('/api/stats/<scenario>')
def api_stats(scenario):
    return jsonify(get_scenario_stats(scenario))

@app.route('/api/upload', methods=['POST'])
def api_upload():
    if 'file' not in request.files:
        return jsonify({'error': 'No file'}), 400
    
    file = request.files['file']
    scenario_name = request.form.get('scenario', 'custom')
    
    filepath = os.path.join('uploads', file.filename)
    os.makedirs('uploads', exist_ok=True)
    file.save(filepath)
    
    if analyze_log_file(filepath, scenario_name):
        monitor.uploaded_files.append({
            'filename': file.filename,
            'scenario': scenario_name,
            'timestamp': datetime.now().isoformat()
        })
        return jsonify({'success': True, 'stats': get_scenario_stats(scenario_name)})
    else:
        return jsonify({'error': 'Analysis failed'}), 500

@socketio.on('connect')
def handle_connect():
    emit('scenarios_list', {'scenarios': list(monitor.scenarios.keys())})

if __name__ == '__main__':
    import sys
    logfile = sys.argv[1] if len(sys.argv) > 1 else 'xv6_output.log'
    
    monitor_thread = threading.Thread(target=monitor_xv6_output, args=(logfile,), daemon=True)
    monitor_thread.start()
    
    print("=" * 70)
    print("Enhanced Priority Inheritance Monitor")
    print("=" * 70)
    print(f"Dashboard: http://localhost:5000")
    print(f"Analysis: http://localhost:5000/analysis/pi_detailed")
    print(f"Compare: http://localhost:5000/compare")
    print(f"Upload: http://localhost:5000/upload")
    print("=" * 70)
    
    socketio.run(app, host='0.0.0.0', port=5000, debug=True, use_reloader=False)
