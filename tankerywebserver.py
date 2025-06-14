from flask import Flask, Response
from flask_cors import CORS

app = Flask(__name__)
cors = CORS(app)

@app.route('/', methods=['GET'])
def tanki():
    resp = Response(open("cmake-build-debug-emsdk//TankeryClient.html").read())
    resp.headers['Cross-Origin-Embedder-Policy'] = 'require-corp'
    resp.headers['Cross-Origin-Opener-Policy'] = 'same-origin'
    return resp

@app.route('/TankeryClient.js', methods=['GET'])
def getjs():
    resp = Response(open("cmake-build-debug-emsdk//TankeryClient.js").read(), mimetype="application/javascript")
    resp.headers['Cross-Origin-Embedder-Policy'] = 'require-corp'
    resp.headers['Cross-Origin-Opener-Policy'] = 'same-origin'
    return resp

@app.route('/TankeryClient.wasm', methods=['GET'])
def getwasm():
    resp = Response(open("cmake-build-debug-emsdk//TankeryClient.wasm", "rb").read(), mimetype="application/wasm")
    resp.headers['Cross-Origin-Embedder-Policy'] = 'require-corp'
    resp.headers['Cross-Origin-Opener-Policy'] = 'same-origin'
    return resp

@app.route('/TankeryClient.data', methods=['GET'])
def getdata():
    resp = Response(open("cmake-build-debug-emsdk//TankeryClient.data", "rb").read(), mimetype="application/octet-stream")
    resp.headers['Cross-Origin-Embedder-Policy'] = 'require-corp'
    resp.headers['Cross-Origin-Opener-Policy'] = 'same-origin'
    return resp

app.run(host="localhost", port=5000, threaded=False, debug=True)