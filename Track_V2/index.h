const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Datalogger</title>
    <style>
        html {
            font-family: Arial, Helvetica, sans-serif;
        }

        body {
            background-color: #f4f4f4;
            margin: 0;
            padding: 0;
        }

        .container {
            max-width: 800px;
            margin: 50px auto;
            text-align: center;
        }

        h1 {
            color: #333;
        }

        .button {
            display: inline-block;
            padding: 10px 20px;
            margin: 10px;
            font-size: 16px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            transition-duration: 0.4s;
        }

        .button-data {
            background-color: #858585;
            color: #fff;
        }

        .button-delete {
            background-color: #780320;
            color: #fff;
        }

        .button:hover {
            background-color: #0056b3;
        }

        table {
            width: 100%;
            margin: 20px 0;
            border-collapse: collapse;
        }

        th, td {
            padding: 10px;
            border: 1px solid #ddd;
            text-align: center;
        }

        th {
            background-color: #f2f2f2;
        }

    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Datalogger - Manage Data</h1>
        <h2>Files in Track Folder</h2>
        %FILE_LIST% <!-- Placeholder for the file table -->
    </div>
</body>
</html>
)rawliteral";
