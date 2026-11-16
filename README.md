# ALF-EWG: Autonomous Landslide & Flash-Flood Early Warning Grid

**Course:** Object-Oriented Programming Using C++ (CIC-211)
**Institute:** Maharaja Agrasen Institute of Technology, Dept. of CSE
**Project:** Option 1 — ALF-EWG
**Submitted by:** `<Student Name 1> (Enrollment No.)`, `<Student Name 2> (Enrollment No.)`
**Submission date:** `<date>` (deadline: 27 November 2026, 11:59 PM)

---

## 1. Overview

ALF-EWG simulates the central processing backend of an early warning system for landslides and flash floods in mountainous terrain. A grid of solar-powered IoT sensor nodes (river sonar gauges, soil-moisture probes, seismic geophones, acoustic tiltmeters) is swept cyclically. Each node reports telemetry and checks its own warning threshold. Incidents are sorted by hazard priority and persisted to disk using a transactional binary log with checksum verification and rollback.

Sensor readings are **simulated** (hard-coded values) because no physical hardware is attached. The focus is on the object-oriented design.

## 2. Build and Run

Requirements: any C++11 / C++14 / C++17 compiler (tested with g++).

```bash
g++ -std=c++17 -Wall -Wextra main.cpp -o alf
./alf          # Linux / macOS
alf.exe        # Windows
```

The program is a single file, `main.cpp`. It writes two files in the working directory:

| File | Purpose |
|---|---|
| `disaster_logs.dat` | Binary incident log (transactional) |
| `stations.txt` | Station states written through a file stream |

`disaster_logs.dat` is deleted at the start of Phase 6, so every run starts with a clean log.

## 3. Program Structure

```
DisasterCore
├── Sensors
│   ├── GeoLocation, updateCoordinates(), accumulateTelemetry()
│   ├── DisasterTelemetryDevice (virtual root)
│   │   ├── SolarPoweredNode            (virtual inheritance)
│   │   ├── WirelessLoRaTransceiver     (virtual inheritance)
│   │   └── AutonomousHybridProbe       (diamond leaf)
│   ├── PowerSubsystem                  (contained object)
│   ├── BaseSensorNode  (abstract)
│   │   ├── RiverSonarGauge
│   │   ├── SoilMoistureProbe
│   │   ├── SeismicGeophone
│   │   └── AcousticTiltmeter
│   ├── TelemetryBuffer, SensorStation, EarlyWarningSiren
│   ├── SecureSensorData  <--friend--  TelemetryAuditor
│   ├── SensorFailureException, TelemetryChecksumMismatch, ThresholdOverflowException
│   └── IncidentRecord, PriorityEvacuationRouter, DisasterLogger
└── Alerts
    ├── calculateBaseRisk(), computeHazardIndex() x2   (inline, overloaded)
    └── alertHeader, AlertRow                           (stream manipulators)
```

## 4. Rubric Compliance Map

### Unit I: Core Foundations & Call Semantics

| Requirement | Implementation | `main.cpp` line |
|---|---|---|
| Namespaces `DisasterCore::Sensors` and `DisasterCore::Alerts` | Two nested namespaces inside `DisasterCore` | 14, 17, 395 |
| Inline risk-indexing functions | `calculateBaseRisk`, `computeHazardIndex` (both overloads) are `inline` | 397–405 |
| Overloaded `computeHazardIndex()` | `(double rainfall, double slope)` and `(double soilSaturation, int vibrationFreq)` | 399, 403 |
| Call by reference + return by reference | `GeoLocation& updateCoordinates(GeoLocation&, ...)` and `double& accumulateTelemetry(double&, double)`; the latter is used as an lvalue in `main` (`accumulateTelemetry(total, 4.2) += 1.0;`) | 27, 33, 443 |
| Friend class inspecting private data | `TelemetryAuditor` is a friend of `SecureSensorData` and reads `rawSeismicVoltage` and `isCalibrated` with no getters | 314–325 |

### Unit II: Encapsulation, Dynamic Memory & Operator Overloading

| Requirement | Implementation | `main.cpp` line |
|---|---|---|
| Entities `SensorStation`, `EarlyWarningSiren`, `TelemetryBuffer` with data abstraction and const correctness | Private data, `const` getters and `const` operators | 183, 254, 274 |
| Static data members | `EarlyWarningSiren::globalAlertLevel`, `SensorStation::activeStations` | 256, 276 |
| Static member functions | `getGlobalAlertLevel()`, `resetGlobalAlertLevel()`, `getActiveStations()` | 269, 289 |
| Parameterized / dynamic constructor | `TelemetryBuffer(int, int)` allocates with `new int[size]` | 192 |
| Deep copy constructor | `TelemetryBuffer(const TelemetryBuffer&)` | 198 |
| Explicit destructor | `~TelemetryBuffer()` calls `delete[]` | 218 |
| Copy assignment (Rule of Three) | `operator=` with self-assignment check and exception-safe copy | 206 |
| Binary `+` aggregates clusters | `operator+` merges two buffers' contents and sums their hazard scores | 220 |
| Relational `>` prioritizes hazard | `TelemetryBuffer::operator>` and `SensorStation::operator>` | 227, 288 |
| Stream insertion `<<` | `TelemetryBuffer` and `SensorStation` (console and file streams) | 231, 291 |
| Stream extraction `>>` | `TelemetryBuffer` and `SensorStation` (demonstrated with `std::istringstream`) | 237, 294 |

### Unit III: Inheritance, Containership & Polymorphism

| Requirement | Implementation | `main.cpp` line |
|---|---|---|
| Abstract `BaseSensorNode` with pure virtuals | `readTelemetry() = 0` and `evaluateThreshold() = 0` | 81 |
| Diamond problem via virtual base classes | Root `DisasterTelemetryDevice` → `SolarPoweredNode` and `WirelessLoRaTransceiver` (both `virtual public`) → `AutonomousHybridProbe` | 39–65 |
| Containership (aggregation) | `PowerSubsystem powerUnit` is a member object inside `BaseSensorNode` (has-a, not is-a) | 74, 81 |
| `std::vector<BaseSensorNode*>` with runtime dispatch | Phase 3 sweeps four different sensor types through base pointers over 2 cycles | `main()` Phase 3 |
| Virtual destructors | `BaseSensorNode` and `DisasterTelemetryDevice` have virtual destructors, so `delete` through a base pointer is safe | 81, 39 |

### Unit IV: Streams, Persistence, Exception Safety & STL

| Requirement | Implementation | `main.cpp` line |
|---|---|---|
| Custom exceptions from `std::exception` with custom `what()` | `SensorFailureException`, `TelemetryChecksumMismatch`, `ThresholdOverflowException` | 93, 101, 111 |
| Binary `std::fstream` logging | `DisasterLogger::logIncidentsToDisk` writes `IncidentRecord` structs with `write()` | 340 |
| Transactional rollback with `seekp()` / `seekg()` | Snapshot the old file → append new records (`seekp`) → read back (`seekg`) → compare checksum → **commit** or **restore the snapshot and throw** `TelemetryChecksumMismatch` | 340–390 |
| `std::vector` | Sensor grid; incident list | `main()` |
| `std::map` | `zoneHazard` aggregates hazard score per region | 539 |
| `std::sort` with custom functor | `PriorityEvacuationRouter` orders incidents by descending hazard | 332, 355 |
| `std::for_each` | Used while writing records, and when printing the alert table | 361, 543 |
| Custom stream manipulators | `Alerts::alertHeader` (no-argument manipulator) and `Alerts::AlertRow` (parameterized, formatted via `operator<<`) | 408, 413 |

## 5. Key Design Decisions

- **Virtual inheritance:** `AutonomousHybridProbe` initializes the shared `DisasterTelemetryDevice` directly in its own constructor initializer list. With virtual bases, the most-derived class is responsible for constructing the virtual base, so only one base subobject (and one `deviceID`) exists.
- **Containership vs. inheritance:** A sensor node *has a* power subsystem but *is not* one, so `PowerSubsystem` is composed inside `BaseSensorNode`.
- **Rule of Three:** `TelemetryBuffer` owns raw heap memory, so it defines a destructor, a deep copy constructor and a copy assignment operator. `SensorStation` holds a `TelemetryBuffer` by value and relies on those.
- **Transaction safety:** The logger never leaves the file half-written after a failed verification. It restores the byte-for-byte pre-transaction snapshot.
- **Exception handling in the sweep:** Each node is wrapped in its own `try/catch`. When the tiltmeter throws `ThresholdOverflowException`, the sweep continues with the other nodes.
- **Binary log portability:** Records are written as raw structs. The file is therefore not portable across different compilers or architectures, which is acceptable for this scope.

## 6. Sample Output (abridged)

```
--- ALF-EWG Phase 1: Core Foundations ---
Updated Location: Lat 29.44, Lon 78.16
Hazard Index (Rain & Slope): 41.25
Hazard Index (Soil & Vibration): 223.5
Accumulated telemetry: 5.2

--- ALF-EWG Phase 2: Diamond Problem Resolution ---
-> Base Device Powered ON. ID: 2725        <- printed once
-> Solar Panels Deployed.
-> LoRa Network Connected.
=> Autonomous Hybrid Probe is fully Operational!

--- ALF-EWG Phase 3: ... Polymorphic Sweep ---
Sweep cycle 1:
[Power: 100%] Ganga_Node_1 -> Reading River Level: 15.5 meters
  => ALERT! Flash Flood Warning at Ganga_Node_1
...
  EXCEPTION: OVERFLOW at Tilt_Node_4: reading 95.000000 exceeds hard limit 90.000000

--- ALF-EWG Phase 4: Memory & Operator Overloading ---
Combined Regional Buffer State: [Buffer Capacity: 80 units | Hazard Score: 130]
Active stations (with copy): 3
Active stations (after scope): 2

--- ALF-EWG Phase 6: STL & File Persistence ---
Transaction committed: 3 incidents saved to disaster_logs.dat.
NODE              LEVEL       VALUE
----------------------------------------
ZONE_ALPHA        WARNING     45.00
ZONE_BETA         CRITICAL    120.00
ZONE_GAMMA        WARNING     80.00
ROLLED BACK: CHECKSUM MISMATCH: expected 246 but disk returned 245
```

(The exception lines go to `stderr`, so their position in the terminal may differ slightly.)

## 7. Testing Notes

- Compiled cleanly with `g++ -std=c++17 -Wall -Wextra` (no warnings) and passes a `-std=c++11` syntax check.
- Run under AddressSanitizer and UndefinedBehaviorSanitizer with no errors or leaks reported.
- The rollback path is exercised by calling `logIncidentsToDisk(..., true)`, which injects a deliberate checksum fault.

## 8. Limitations and Future Work

- Sensor values are simulated, not read from hardware.
- The station `<<` output is human-readable, while `>>` expects the compact form `NAME SIZE SCORE`.
- A real deployment would add a text/JSON log format, a configurable threshold per node, and multithreaded sensor ingestion.
