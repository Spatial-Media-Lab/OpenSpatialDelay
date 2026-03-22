#!/usr/bin/env python3
"""
OSC Testing Tool for OpenSpatialDelay v1.0

Tests both ADM-OSC standard (/adm/obj/N/) and OSD custom (/osd/obj/N/, /osd/global/) namespaces.

Dependencies:
    pip install python-osc

Usage:
    # --- Position (ADM-OSC standard) ---
    python scripts/adm_osc_test.py --manual 1 45.0 10.0 0.5
    python scripts/adm_osc_test.py --orbit 1
    python scripts/adm_osc_test.py --sweep 3
    python scripts/adm_osc_test.py --multi
    python scripts/adm_osc_test.py --xyz 1 0.5 0.5 0.0
    python scripts/adm_osc_test.py --aed 1 90.0 20.0 0.8
    python scripts/adm_osc_test.py --axes 1

    # --- Per-object params (/osd/obj/N/) ---
    python scripts/adm_osc_test.py --obj 1 enabled 1
    python scripts/adm_osc_test.py --obj 1 doppler 0.8
    python scripts/adm_osc_test.py --obj 1 pitch -12
    python scripts/adm_osc_test.py --obj 1 trajectory 9
    python scripts/adm_osc_test.py --obj 1 speed 2.5
    python scripts/adm_osc_test.py --obj 1 direction 1
    python scripts/adm_osc_test.py --obj 1 input 1

    # --- Position via /osd/ alias ---
    python scripts/adm_osc_test.py --obj 1 azim 90.0
    python scripts/adm_osc_test.py --obj 1 aed 45.0 10.0 0.5

    # --- Global params (/osd/global/) ---
    python scripts/adm_osc_test.py --global feedback 0.8
    python scripts/adm_osc_test.py --global delaytime 250
    python scripts/adm_osc_test.py --global drywet 0.7
    python scripts/adm_osc_test.py --global air 1

    # --- Global tap offsets (/osd/global/tap*) ---
    python scripts/adm_osc_test.py --global tapazimuth 45
    python scripts/adm_osc_test.py --global tapelevation -15
    python scripts/adm_osc_test.py --global tapdistance 0.3
    python scripts/adm_osc_test.py --global tapdoppler 0.5
    python scripts/adm_osc_test.py --global tappitch -12
    python scripts/adm_osc_test.py --global tapspeed 1.5

    # --- Listen for OSC Send ---
    python scripts/adm_osc_test.py --listen
    python scripts/adm_osc_test.py --listen --listen-port 4003

    # --- Options ---
    python scripts/adm_osc_test.py --orbit 1 --port 4002
    python scripts/adm_osc_test.py --orbit 1 --rate 60
"""

import argparse
import math
import sys
import time

try:
    from pythonosc import udp_client
    from pythonosc import dispatcher as osc_dispatcher
    from pythonosc import osc_server
except ImportError:
    print("Error: python-osc not installed. Run: pip install python-osc")
    sys.exit(1)


def send_manual(client, obj, az, el, dist):
    """Send a single /adm/obj/N/aed message."""
    addr = f"/adm/obj/{obj}/aed"
    client.send_message(addr, [az, el, dist])
    print(f"Sent {addr} [{az:.1f}, {el:.1f}, {dist:.3f}]")


def send_xyz(client, obj, x, y, z):
    """Send a single /adm/obj/N/xyz message."""
    addr = f"/adm/obj/{obj}/xyz"
    client.send_message(addr, [x, y, z])
    print(f"Sent {addr} [{x:.3f}, {y:.3f}, {z:.3f}]")


def send_aed(client, obj, az, el, dist):
    """Send a single /adm/obj/N/aed message (same as manual, explicit name)."""
    send_manual(client, obj, az, el, dist)


def run_orbit(client, obj, speed, rate):
    """Continuously orbit object around azimuth axis."""
    print(f"Orbiting object {obj} at {speed:.1f} rev/sec (Ctrl+C to stop)")
    dt = 1.0 / rate
    phase = 0.0
    try:
        while True:
            az = -180.0 + 360.0 * phase
            addr = f"/adm/obj/{obj}/azim"
            client.send_message(addr, [az])
            phase += speed * dt
            phase -= math.floor(phase)
            time.sleep(dt)
    except KeyboardInterrupt:
        print("\nStopped.")


def run_sweep(client, obj, rate):
    """Sweep object through full elevation range (-90 to +90 and back)."""
    print(f"Sweeping object {obj} elevation (Ctrl+C to stop)")
    dt = 1.0 / rate
    phase = 0.0
    speed = 0.25  # 1 full sweep cycle per 4 seconds
    try:
        while True:
            # Triangle wave: 0->1->0 over one cycle
            tri = 1.0 - abs(2.0 * phase - 1.0)
            el = -90.0 + 180.0 * tri
            addr = f"/adm/obj/{obj}/elev"
            client.send_message(addr, [el])
            phase += speed * dt
            phase -= math.floor(phase)
            time.sleep(dt)
    except KeyboardInterrupt:
        print("\nStopped.")


def run_multi(client, rate):
    """Coordinated movement of all 12 objects in a spiral pattern."""
    print("Moving all 12 objects in coordinated spiral (Ctrl+C to stop)")
    dt = 1.0 / rate
    phase = 0.0
    speed = 0.2  # Slow coordinated movement
    try:
        while True:
            for obj in range(1, 13):
                # Each object offset by 30 degrees (360/12) in azimuth
                obj_phase = phase + (obj - 1) / 12.0
                obj_phase -= math.floor(obj_phase)
                az = -180.0 + 360.0 * obj_phase
                el = 30.0 * math.sin(obj_phase * 2 * math.pi)
                dist = 0.3 + 0.4 * (0.5 + 0.5 * math.cos(obj_phase * 2 * math.pi))
                client.send_message(f"/adm/obj/{obj}/aed", [az, el, dist])

            phase += speed * dt
            phase -= math.floor(phase)
            time.sleep(dt)
    except KeyboardInterrupt:
        print("\nStopped.")


def run_individual_axes(client, obj, rate):
    """Send individual /x, /y, /z messages (tests partial Cartesian handling)."""
    print(f"Sending individual x/y/z to object {obj} (Ctrl+C to stop)")
    dt = 1.0 / rate
    phase = 0.0
    speed = 0.5
    try:
        while True:
            x = 0.5 * math.sin(phase * 2 * math.pi)
            y = 0.5 * math.cos(phase * 2 * math.pi)
            z = 0.2 * math.sin(phase * 4 * math.pi)
            client.send_message(f"/adm/obj/{obj}/x", [x])
            client.send_message(f"/adm/obj/{obj}/y", [y])
            client.send_message(f"/adm/obj/{obj}/z", [z])
            phase += speed * dt
            phase -= math.floor(phase)
            time.sleep(dt)
    except KeyboardInterrupt:
        print("\nStopped.")


def send_obj_param(client, obj, param, values):
    """Send /osd/obj/N/<param> message."""
    addr = f"/osd/obj/{obj}/{param}"
    float_vals = [float(v) for v in values]
    client.send_message(addr, float_vals)
    if len(float_vals) == 1:
        print(f"Sent {addr} [{float_vals[0]}]")
    else:
        print(f"Sent {addr} {float_vals}")


def send_global_param(client, param, value):
    """Send /osd/global/<param> message."""
    addr = f"/osd/global/{param}"
    val = float(value)
    client.send_message(addr, [val])
    print(f"Sent {addr} [{val}]")


def run_listen(port):
    """Listen for incoming OSC messages (ADM-OSC + OSD) and print them."""
    print(f"Listening for OSC messages on port {port} (Ctrl+C to stop)")
    msg_count = [0]

    def handle_aed(address, *args):
        msg_count[0] += 1
        parts = address.split("/")
        obj = parts[3] if len(parts) >= 5 else "?"
        if len(args) >= 3:
            print(f"  [{msg_count[0]:6d}] obj {obj:>2s}  az={args[0]:+8.2f}  el={args[1]:+7.2f}  dist={args[2]:.3f}")
        else:
            print(f"  [{msg_count[0]:6d}] {address} {list(args)}")

    def handle_osd_obj(address, *args):
        msg_count[0] += 1
        parts = address.split("/")
        obj = parts[3] if len(parts) >= 5 else "?"
        prop = parts[4] if len(parts) >= 5 else "?"
        vals = [f"{a:.3f}" if isinstance(a, float) else str(a) for a in args]
        print(f"  [{msg_count[0]:6d}] obj {obj:>2s}  {prop}={', '.join(vals)}")

    def handle_osd_global(address, *args):
        msg_count[0] += 1
        prop = address.split("/")[-1]
        vals = [f"{a:.3f}" if isinstance(a, float) else str(a) for a in args]
        print(f"  [{msg_count[0]:6d}] global  {prop}={', '.join(vals)}")

    def handle_default(address, *args):
        msg_count[0] += 1
        print(f"  [{msg_count[0]:6d}] {address} {list(args)}")

    d = osc_dispatcher.Dispatcher()
    d.map("/adm/obj/*/aed", handle_aed)
    d.map("/osd/obj/*", handle_osd_obj)
    d.map("/osd/global/*", handle_osd_global)
    d.set_default_handler(handle_default)

    server = osc_server.BlockingOSCUDPServer(("0.0.0.0", port), d)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print(f"\nStopped. Received {msg_count[0]} messages.")
        server.server_close()


def main():
    parser = argparse.ArgumentParser(
        description="OSC Testing Tool for OpenSpatialDelay v1.0",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )

    parser.add_argument("--port", type=int, default=4002, help="OSC target port for sending (default: 4002)")
    parser.add_argument("--host", type=str, default="127.0.0.1", help="OSC target host (default: 127.0.0.1)")
    parser.add_argument("--rate", type=int, default=60, help="Send rate in Hz for continuous modes (default: 60)")
    parser.add_argument("--listen-port", type=int, default=4003, help="Port to listen on in --listen mode (default: 4003)")

    # Mutually exclusive test modes
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--manual", nargs=4, metavar=("OBJ", "AZ", "EL", "DIST"),
                       help="Send single AED position: object# azimuth elevation distance")
    group.add_argument("--orbit", nargs="+", metavar="ARG",
                       help="Orbit object: OBJ [SPEED] (default speed: 1.0 rev/sec)")
    group.add_argument("--sweep", type=int, metavar="OBJ",
                       help="Sweep object through elevation range")
    group.add_argument("--multi", action="store_true",
                       help="Coordinated movement of all 12 objects")
    group.add_argument("--xyz", nargs=4, metavar=("OBJ", "X", "Y", "Z"),
                       help="Send Cartesian position: object# x y z")
    group.add_argument("--aed", nargs=4, metavar=("OBJ", "AZ", "EL", "DIST"),
                       help="Send combined AED message: object# azimuth elevation distance")
    group.add_argument("--axes", type=int, metavar="OBJ",
                       help="Send individual /x, /y, /z messages (test partial Cartesian)")
    group.add_argument("--obj", nargs="+", metavar="ARG",
                       help="Per-object param: OBJ PARAM VALUE [VALUE...] (e.g., --obj 1 doppler 0.8)")
    group.add_argument("--global", nargs=2, metavar=("PARAM", "VALUE"), dest="global_param",
                       help="Global param: PARAM VALUE (e.g., --global feedback 0.8)")
    group.add_argument("--listen", action="store_true",
                       help="Listen for OSC messages from the plugin")

    args = parser.parse_args()

    if args.listen:
        run_listen(args.listen_port)
        return

    client = udp_client.SimpleUDPClient(args.host, args.port)
    print(f"OSC Test -> {args.host}:{args.port}")

    if args.manual:
        obj, az, el, dist = int(args.manual[0]), float(args.manual[1]), float(args.manual[2]), float(args.manual[3])
        send_manual(client, obj, az, el, dist)

    elif args.orbit:
        obj = int(args.orbit[0])
        speed = float(args.orbit[1]) if len(args.orbit) > 1 else 1.0
        run_orbit(client, obj, speed, args.rate)

    elif args.sweep is not None:
        run_sweep(client, args.sweep, args.rate)

    elif args.multi:
        run_multi(client, args.rate)

    elif args.xyz:
        obj, x, y, z = int(args.xyz[0]), float(args.xyz[1]), float(args.xyz[2]), float(args.xyz[3])
        send_xyz(client, obj, x, y, z)

    elif args.aed:
        obj, az, el, dist = int(args.aed[0]), float(args.aed[1]), float(args.aed[2]), float(args.aed[3])
        send_aed(client, obj, az, el, dist)

    elif args.axes is not None:
        run_individual_axes(client, args.axes, args.rate)

    elif args.obj:
        if len(args.obj) < 3:
            parser.error("--obj requires at least 3 args: OBJ PARAM VALUE [VALUE...]")
        obj = int(args.obj[0])
        param = args.obj[1]
        values = args.obj[2:]
        send_obj_param(client, obj, param, values)

    elif args.global_param:
        param, value = args.global_param
        send_global_param(client, param, value)


if __name__ == "__main__":
    main()
