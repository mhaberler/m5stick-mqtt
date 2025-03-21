from pyzbar.pyzbar import decode
from PIL import Image
import pyzstd
import base64
import json
import sys
import os

# pip install pyzbar pyzstd pillow
# see also: https://stackoverflow.com/questions/71984213/macbook-m1raise-importerrorunable-to-find-zbar-shared-library-importerror
# https://stackoverflow.com/questions/48792965/importerror-unable-to-find-zbar-shared-library-on-flask
# https://stackoverflow.com/questions/63217735/import-pyzbar-pyzbar-unable-to-find-zbar-shared-library
"""
decode qr code image or config code
examples: 
    python decode.py sensorlogger-qrcode.png
    python decode.py 'https://sensorlogger.app/link/config/KLUv/WDaAE0HAFKOKyFQS9sGA+Nlq70JnOSoBduifGLQyKlS7RizvjmAx8xMrwGtlmFNLeccn4LHFACWZNguZhw+HkGwPD0fw7LSOV3HTYVCW0D6Yk+bWx8voCi9+o7vXlgo20Q0qxVzvGqVOJCSEgMhBkqMZAA5jpTe1jm+7ub4PMlNJTSt2um0w/E5czZmbU+nzdNd7MApa7XU4PgghJB8fExnOz2RjVEuNzsXuurwhvRs1Qir7JLfEBUgMIJH5Q4sbH8050KxZ2uwcDtn89HidaiRypmoHtTaggEktmQi4QsQjO59s6M5bITBVF8rrAEP'

"""

prefix = b"https://sensorlogger.app/link/config/"

for arg in sys.argv[1:]:
    if os.path.isfile(arg):
        q = decode(Image.open(arg))
        s = q[0].data
    else:
        s = arg
    print(s)
    cf = s[len(prefix) :]
    d = base64.b64decode(cf)
    cfg = pyzstd.decompress(d)
    js = json.loads(cfg)
    print(json.dumps(js, indent=4))
    #print(json.dumps(js))
