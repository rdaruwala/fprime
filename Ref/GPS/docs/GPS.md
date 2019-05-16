<title>GPS Component Dictionary</title>
# GPS Component Dictionary


## Telemetry Channel List

|Channel Name|ID|Type|Description|
|---|---|---|---|
|GPS_Time|0 (0x0)|U32|UTC Time|
|GPS_Latitude|1 (0x1)|F32|GPS Latitude Reading|
|GPS_Longitude|2 (0x2)|F32|GPS Longitude Reading|
|GPS_Satellites|3 (0x3)|U8|Number of Satellites Acquired|
|GPS_Altitude|4 (0x4)|F32|GPS Altitude|

## Event List

|Event Name|ID|Description|Arg Name|Arg Type|Arg Size|Description
|---|---|---|---|---|---|---|
|GPS_OpenError|0 (0x0)|GPS Serial Open Error| | | | |
|GPS_ReadError|1 (0x1)|GPS Serial Read Error| | | | |
|GPS_ParseError|2 (0x2)|GPS Parse Issues| | | | |
|GPS_LockAcquired|3 (0x3)|GPS Lock Acquired| | | | |
|GPS_LockLost|4 (0x4)|GPS Lock Lost| | | | |
