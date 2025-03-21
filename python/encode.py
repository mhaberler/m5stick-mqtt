from PIL import Image
import pyzstd
import base64
import json
import qrcode

"""
encode a sensorlogger configuration into an QRcode image
example: python encode.py
view generated.png
"""

qr = qrcode.QRCode(
    version=1,
    error_correction=qrcode.constants.ERROR_CORRECT_L,
    box_size=10,
    border=4,
)

prefix =  b"https://sensorlogger.app/link/config/"

cfg = {
    "sensorState": {
        "Accelerometer": {"enabled": True},
        "Gravity": {"enabled": False},
        "Gyroscope": {"enabled": True},
        "Orientation": {"enabled": True},
        "Magnetometer": {"enabled": True},
        "Barometer": {"enabled": True},
        "Location": {"enabled": False},
        "Microphone": {"enabled": False},
        "Camera": {"enabled": False},
        "Battery": {"enabled": False},
        # "Heart Rate": {
        #     "enabled": False
        # },
        # "Wrist Motion": {
        #     "enabled": False
        # },
        "bluetooth-54edede7-44ae-d2ab-c0a4-21c34d29d909": {"enabled": True},
        "bluetooth-6ec7c2f2-bd98-5e88-19ed-776a439185ea": {"enabled": True},
        "bluetooth-ca54186c-0c76-1554-a9f8-bff861dcf32f": {"enabled": True},
        "bluetooth-42e60228-c5ff-8c32-b353-db0457890208": {"enabled": False},
        "bluetooth-4595d372-d03d-d413-8c73-81747c65db19": {"enabled": False},
        "bluetooth-d4092e48-262d-fe8c-db1e-92ec4a46f15c": {"enabled": False},
        "bluetooth-17456af3-1abc-b70d-4363-2579d4c23609": {"enabled": False},
        "bluetooth-60a13697-013a-c545-e423-09b5e2b21e87": {"enabled": True},
        "bluetooth-9a7602d0-a3ab-51b3-b27d-372cb3f16b64": {"enabled": True},
        "bluetooth-f76afde0-54d9-036e-dfdb-a3cc759fe462": {"enabled": True},
        "bluetooth-fdcd84a0-93ea-01b3-72fc-87e4de4f2b26": {"enabled": True},
        "bluetooth-aa06218b-d30a-9150-b69e-866cec951617": {"enabled": True},
        "bluetooth-536ef24e-38ea-8250-d9f9-e71f305c8171": {"enabled": False},
        "bluetooth-e7fb8eb3-91ff-8123-5cec-d754d37a8b6e": {"enabled": True},
        "bluetooth-575e5884-beb2-a4c5-1aad-14153c1c3172": {"enabled": False},
        "Brightness": {"enabled": False},
        "bluetooth-e542101a-2a02-1c6e-7c28-c91fcc03a7b2": {"enabled": False},
    },
    "http": {
        "enabled": True,
        "url": "https://foobar.com/push",
        "batchPeriod": 1000,
    },
    "additionalLocation": True,
    "fileFormat": ".json",
    "fileName": "DATETIME_LOCAL_FORMATTED-RECORDING_NAME",
}

s = json.dumps(cfg).encode()
z = pyzstd.compress(s)
e = prefix + base64.b64encode(z)
qr.add_data(e)
qr.make(fit=True)
img = qr.make_image(fill_color="black", back_color="white")
img.save("generated.png")
