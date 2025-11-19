# Map Address

![Screenshot](./mapaddress.png)

A C++/Qt application for managing and visualizing physical addresses on
interactive maps with OpenStreetMap and Google Maps support.

Address Management:

- Multiple Address Lists: Create, rename, and delete multiple address lists
- CRUD Operations: Add, edit, and delete addresses with dw0full geocoding support
- Search & Filter: Real-time search through addresses
- CSV Import/Export: Bulk import and export address data
- Address Details: View address information with coordinates

Map Integration:

- Dual Map Providers: Switch between Google Maps and OpenStreetMap
- Interactive Markers: Custom markers with tooltips and popups
- Address Geocoding: Automatic address-to-coordinates conversion using
  OpenStreetMap Nominatim
- Reverse Geocoding: Click map to add addresses at any location
- Map Navigation: Zoom, pan, and fit all markers functionality
- Bidirectional Sync: Click markers to select addresses or click
  addresses to center map
- Routing: Mark the start and the end the generate a route to visit all
  addresses included in a list

Data Persistence:

- SQLite Database: Automatic saving with persistent storage
- Auto-save: All changes saved immediately
- Data Integrity: Foreign key constraints and cascade deletes

User Interface:

- Intuitive UI: Built with Qt Designer UI files
- Sidebar Navigation: Address lists and search panel
- Toolbar Actions: Quick access to common operations
- Status Updates: Real-time feedback for all operations
- Address Details Panel: View full information on selection

Error Handling & Logging:

- Comprehensive Error Handling: Network, database, and validation errors
- User-Friendly Messages: Clear error descriptions
- Logging System: File and console logging with configurable levels
- Input Validation: Format checking and required field validation

Build Requirements:

- CMake: 3.16 or higher
- Qt: 6.x
  - Qt Core
  - Qt Widgets
  - Qt Network
  - Qt WebEngineWidgets
  - Qt WebChannel
  - Qt Location
  - Qt Positioning
  - Qt Sql
  - Qt Test (for running tests)
- C++17: Compatible compiler (GCC, Clang, MSVC)

Runtime Requirements:

- Internet connection for geocoding and map tiles
- Optional: Google Maps API key (set via `GOOGLE_MAPS_API_KEY`
  environment variable)

Building:

Standard Build

```bash
mkdir build
cd build
cmake ..
make
```

Build with Tests

```bash
mkdir build
cd build
cmake ..
make
make test  # or: ctest
```

Install:

```bash
sudo make install
```

Running:

Basic Usage

```bash
./MapAddress
```

With Google Maps API Key:

```bash
export GOOGLE_MAPS_API_KEY="your-api-key-here"
./MapAddress
```

Debugging:

Web Console Debugging: The application includes built-in web console
debugging for the map widget.

Method 1: Remote Debugging (Chrome DevTools)

1. Run the application
2. Open Chrome, Edge, or any Chromium-based browser
3. Navigate to `chrome://inspect` or `edge://inspect`
4. Look for "Remote Target" - click "inspect" to open DevTools
5. Full JavaScript debugging, console, network monitoring available

Method 2: Console Logs in Application

- JavaScript console messages are automatically captured and logged
- Check application logs at
  `~/.local/share/DataInquiry/MapAddress/mapaddress.log`
- Console output also appears in terminal (stdout) with `[JS INFO]`,
  `[JS WARNING]`, `[JS ERROR]` prefixes

Configuration:

Environment Variables

- `GOOGLE_MAPS_API_KEY`: Optional API key for Google Maps integration

Data Storage

- Database: `~/.local/share/DataInquiry/MapAddress/mapaddress.db` (Linux)
- Logs: `~/.local/share/DataInquiry/MapAddress/mapaddress.log` (Linux)

Creating Address Lists:

1. Click "New" button in the sidebar
2. Enter list name
3. Click OK

Adding Addresses:

Method 1: Traditional Entry

1. Select a list from the dropdown
2. Click "Add" button or press Ctrl+N
3. Enter address in format: `Street, City, State ZIP`
   - Example: `123 Main St, Springfield, IL 62701`
4. Click "Verify Address" to geocode
5. Click OK to save

Method 2: Click on Map

1. Select a list from the dropdown
2. Click anywhere on the map to mark a location
3. Select "Add Address at Location" from the context menu
4. The application will automatically reverse geocode the coordinates
5. Review and edit the address details if needed
6. Click OK to save

The map-click feature is particularly useful for:
- Adding addresses for locations without formal addresses
- Quickly marking points of interest on the map
- Adding addresses in remote or rural areas
- Creating waypoints for route planning

Importing Addresses:

1. File → Import
2. Select CSV file with format: `Street,City,State,ZIP,Country,Latitude,Longitude`
3. Addresses are added to current list

Exporting Addresses:

1. File → Export
2. Choose save location
3. CSV file created with all addresses from current list

Map Operations:

- Switch Providers: Use toolbar dropdown to select Google Maps or OpenStreetMap
- Navigate: Click and drag to pan, scroll to zoom
- Select Address: Click marker on map or address in list
- Fit All: View → Fit All Markers or toolbar button

Testing:

The application includes unit tests for data models,
database operations, and CRUD functionality.

Running Tests

```bash
cd build
ctest --output-on-failure
```

Test Coverage 

- test_address: Address model validation
- test_addresslist: Address list operations
- test_database: Database CRUD and persistence
