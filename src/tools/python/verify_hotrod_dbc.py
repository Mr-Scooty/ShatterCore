#!/usr/bin/env python3
"""
Hot Rod DBC Verification Script
Verifies that the Hot Rod vehicle has been properly fixed in the DBC files
"""

import struct
import sys
import os

VEHICLE_SEAT_FLAG_CAN_CONTROL = 0x00000800
VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT = 0x02000000

def verify_vehicle_dbc(filename):
    """Verify Vehicle.dbc has correct Hot Rod configuration"""
    print(f"\nVerifying Vehicle.dbc: {filename}")
    print("-" * 40)
    
    try:
        with open(filename, 'rb') as f:
            # Read header
            magic = f.read(4)
            if magic != b'WDBC':
                print(f"✗ Invalid DBC format")
                return False
            
            record_count = struct.unpack('I', f.read(4))[0]
            field_count = struct.unpack('I', f.read(4))[0]
            record_size = struct.unpack('I', f.read(4))[0]
            string_block_size = struct.unpack('I', f.read(4))[0]
            
            print(f"Records: {record_count}, Fields: {field_count}, Record Size: {record_size}")
            
            # Search for Hot Rod (ID 181)
            hot_rod_found = False
            for i in range(record_count):
                record_data = f.read(record_size)
                vehicle_id = struct.unpack('I', record_data[0:4])[0]
                
                if vehicle_id == 181:
                    hot_rod_found = True
                    print(f"\n✓ Found Hot Rod (Vehicle ID: 181)")
                    
                    # Check seats (start at byte 24)
                    seats = []
                    for j in range(8):
                        seat_offset = 24 + (j * 4)
                        seat_id = struct.unpack('I', record_data[seat_offset:seat_offset+4])[0]
                        if seat_id != 0:
                            seats.append(seat_id)
                    
                    print(f"  Configured seats: {seats}")
                    
                    # Verify correct seats
                    required_seats = [2081, 2082, 2083, 2084]
                    all_correct = True
                    
                    for idx, required_seat in enumerate(required_seats):
                        if idx < len(seats) and seats[idx] == required_seat:
                            print(f"  ✓ Seat {idx+1}: {required_seat} - Correct")
                        else:
                            actual = seats[idx] if idx < len(seats) else "Empty"
                            print(f"  ✗ Seat {idx+1}: Expected {required_seat}, got {actual}")
                            all_correct = False
                    
                    if all_correct and len(seats) >= 4:
                        print("\n✓✓✓ Hot Rod is CORRECTLY CONFIGURED with all 4 seats!")
                        return True
                    else:
                        print("\n✗✗✗ Hot Rod is NOT correctly configured!")
                        return False
                    break
            
            if not hot_rod_found:
                print("✗ Hot Rod (ID 181) not found in Vehicle.dbc")
                return False
                
    except Exception as e:
        print(f"✗ Error reading file: {e}")
        return False


def verify_vehicleseat_dbc(filename):
    """Verify VehicleSeat.dbc has all required seats"""
    print(f"\nVerifying VehicleSeat.dbc: {filename}")
    print("-" * 40)
    
    try:
        with open(filename, 'rb') as f:
            # Read header
            magic = f.read(4)
            if magic != b'WDBC':
                print(f"✗ Invalid DBC format")
                return False
            
            record_count = struct.unpack('I', f.read(4))[0]
            field_count = struct.unpack('I', f.read(4))[0]
            record_size = struct.unpack('I', f.read(4))[0]
            string_block_size = struct.unpack('I', f.read(4))[0]
            
            print(f"Records: {record_count}, Fields: {field_count}, Record Size: {record_size}")
            
            # Search for seats 2081-2084
            required_seats = {2081: False, 2082: False, 2083: False, 2084: False}
            seat_flags = {}
            
            for i in range(record_count):
                record_data = f.read(record_size)
                seat_id = struct.unpack('I', record_data[0:4])[0]
                
                if seat_id in required_seats:
                    required_seats[seat_id] = True
                    # Get flags (second field)
                    flags = struct.unpack('I', record_data[4:8])[0]
                    seat_flags[seat_id] = flags
            
            print(f"\nSeat verification:")
            all_present = True
            
            seat_names = {
                2081: "Driver seat (controllable)",
                2082: "Passenger seat 1 (Ace)",
                2083: "Passenger seat 2 (Gobber)",
                2084: "Passenger seat 3 (Izzy)"
            }
            
            for seat_id, present in required_seats.items():
                if present:
                    flags = seat_flags.get(seat_id, 0)
                    flags_hex = hex(flags)
                    has_control = bool(flags & VEHICLE_SEAT_FLAG_CAN_CONTROL)
                    can_enter_exit = bool(flags & VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT)
                    expected_control = (seat_id == 2081)
                    seat_ok = True
                    
                    if expected_control and not has_control:
                        seat_ok = False
                        print(f"  ✗ Seat {seat_id}: Present but missing control flag (flags={flags_hex})")
                    elif not expected_control and has_control:
                        seat_ok = False
                        print(f"  ✗ Seat {seat_id}: Present but still has control flag (flags={flags_hex})")
                    elif not can_enter_exit:
                        seat_ok = False
                        print(f"  ✗ Seat {seat_id}: Present but missing enter/exit flag (flags={flags_hex})")
                    else:
                        print(f"  ✓ Seat {seat_id}: Present with correct flags (flags={flags_hex})")
                    
                    if not seat_ok:
                        all_present = False
                else:
                    print(f"  ✗ Seat {seat_id}: MISSING - {seat_names[seat_id]}")
                    all_present = False
            
            if all_present:
                print("\n✓✓✓ All required seats are present in VehicleSeat.dbc!")
                return True
            else:
                print("\n✗✗✗ Some seats are missing from VehicleSeat.dbc!")
                return False
                
    except Exception as e:
        print(f"✗ Error reading file: {e}")
        return False


def main():
    """Main verification function"""
    
    print("=" * 60)
    print("Hot Rod DBC Verification Tool")
    print("Checks if Vehicle ID 181 has all 4 seats configured")
    print("=" * 60)
    
    # Get file paths
    if len(sys.argv) > 1:
        vehicle_file = sys.argv[1]
    else:
        vehicle_file = input("Enter path to Vehicle.dbc: ").strip()
    
    if len(sys.argv) > 2:
        seat_file = sys.argv[2]
    else:
        seat_file = input("Enter path to VehicleSeat.dbc: ").strip()
    
    vehicle_ok = False
    seats_ok = False
    
    # Verify Vehicle.dbc
    if vehicle_file and os.path.exists(vehicle_file):
        vehicle_ok = verify_vehicle_dbc(vehicle_file)
    else:
        print(f"\n✗ Vehicle.dbc not found: {vehicle_file}")
    
    # Verify VehicleSeat.dbc
    if seat_file and os.path.exists(seat_file):
        seats_ok = verify_vehicleseat_dbc(seat_file)
    else:
        print(f"\n✗ VehicleSeat.dbc not found: {seat_file}")
    
    # Final verdict
    print("\n" + "=" * 60)
    print("VERIFICATION RESULTS")
    print("=" * 60)
    
    if vehicle_ok and seats_ok:
        print("\n✓✓✓ SUCCESS! Both DBC files are correctly configured!")
        print("\nThe Hot Rod vehicle should now work properly with all 4 seats:")
        print("- Driver can control the vehicle")
        print("- Ace, Gobber, and Izzy can board as passengers")
        print("\nThe quest 'Rolling with my Homies' should work without crashes!")
    else:
        print("\n✗✗✗ FAILURE! DBC files need to be fixed!")
        print("\nIssues found:")
        if not vehicle_ok:
            print("- Vehicle.dbc: Hot Rod is missing passenger seats")
        if not seats_ok:
            print("- VehicleSeat.dbc: Missing required seat entries")
        print("\nRun fix_hotrod_dbc.py to correct these issues.")
    
    return 0 if (vehicle_ok and seats_ok) else 1


if __name__ == "__main__":
    sys.exit(main())
VEHICLE_SEAT_FLAG_CAN_CONTROL = 0x00000800
VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT = 0x02000000
