#!/bin/bash
# Setup script for xv6 Priority Inheritance Monitor

echo "============================================"
echo "  xv6 Priority Inheritance Monitor Setup"
echo "============================================"
echo ""

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 not found. Please install Python 3.8+"
    exit 1
fi

echo "✓ Python 3 found"

# Install Python dependencies
echo ""
echo "Installing Python dependencies..."
pip3 install flask flask-cors flask-socketio python-socketio

echo ""
echo "============================================"
echo "  Setup Complete!"
echo "============================================"
echo ""
echo "📝 Next steps:"
echo ""
echo "1. Update your xv6 kernel with the modified proc.c:"
echo "   cp proc.c kernel/"
echo "   make clean && make"
echo ""
echo "2. Run xv6 and redirect output to a log file:"
echo "   make qemu | tee xv6_output.log"
echo ""
echo "3. In another terminal, start the monitor server:"
echo "   python3 monitor_server.py xv6_output.log"
echo ""
echo "4. Open your browser to:"
echo "   http://localhost:5000"
echo ""
echo "5. Run your priority inheritance tests in xv6:"
echo "   $ pi_detailed"
echo ""
echo "============================================"
