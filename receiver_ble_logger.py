"""
GastroBot BLE Receiver + Logger (for Appendix E evidence)
- Connects to the capsule (STM32WB)
- Subscribes to pH characteristic notifications
- Decodes packets and logs to CSV with timestamps (Dec 2025-friendly)

Requirements:
    pip install bleak

Usage:
    python receiver_ble_logger.py --name GastroBot --out out.csv
or:
    python receiver_ble_logger.py --addr AA:BB:CC:DD:EE:FF --out out.csv

Fill UUIDs to match your GATT definition.
"""

import argparse
import asyncio
import csv
import datetime as dt
from bleak import BleakClient, BleakScanner

# === Replace with your actual UUIDs ===
PH_SERVICE_UUID = "0000aaaa-0000-1000-8000-00805f9b34fb"
PH_CHAR_UUID    = "0000aaab-0000-1000-8000-00805f9b34fb"

PKT_LEN = 10

def u16_le(b): return int.from_bytes(b, "little", signed=False)
def s16_le(b): return int.from_bytes(b, "little", signed=True)
def u32_le(b): return int.from_bytes(b, "little", signed=False)

def decode(pkt: bytes):
    if len(pkt) != PKT_LEN:
        raise ValueError(f"Unexpected packet length: {len(pkt)} != {PKT_LEN}")

    ts_s   = u32_le(pkt[0:4])
    raw    = u16_le(pkt[4:6])
    vmv    = u16_le(pkt[6:8])
    phx100 = s16_le(pkt[8:10])

    return ts_s, raw, vmv/1000.0, phx100/100.0

async def find_device_by_name(name: str):
    devices = await BleakScanner.discover(timeout=6.0)
    for d in devices:
        if d.name and name.lower() in d.name.lower():
            return d
    return None

async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--name", default=None, help="BLE device name (substring match)")
    ap.add_argument("--addr", default=None, help="BLE address (MAC on some platforms)")
    ap.add_argument("--out", default="gastrobot_log.csv", help="output CSV")
    args = ap.parse_args()

    if not args.addr:
        if not args.name:
            raise SystemExit("Provide --addr or --name")
        dev = await find_device_by_name(args.name)
        if not dev:
            raise SystemExit(f"Device not found by name: {args.name}")
        addr = dev.address
    else:
        addr = args.addr

    print(f"Connecting to {addr} ...")

    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["pc_timestamp", "device_unix_s", "raw_adc", "voltage_V", "pH"])

        last_pkt_id = None
        missed = 0

        def on_notify(_, data: bytearray):
            nonlocal missed, last_pkt_id
            pc_ts = dt.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

            try:
                ts_s, raw, v, ph = decode(bytes(data))
            except Exception as e:
                print("Decode error:", e)
                return

            # Optional: if you include a packet counter in future, compute packet loss here.
            w.writerow([pc_ts, ts_s, raw, f"{v:.4f}", f"{ph:.2f}"])
            f.flush()

            print(pc_ts, "V=", f"{v:.4f}", "pH=", f"{ph:.2f}")

        async with BleakClient(addr) as client:
            await client.start_notify(PH_CHAR_UUID, on_notify)
            print("Subscribed. Press Ctrl+C to stop.")
            while True:
                await asyncio.sleep(1.0)

if __name__ == "__main__":
    asyncio.run(main())
