import subprocess
import base64

import json

try:  # py3
        from shlex import quote
except ImportError:  # py2
        from pipes import quote

from flask import Flask, render_template, redirect, url_for, request

libdir = "./"

def encQuery(query):
    #hkeygen = libdir + "hkeygen"
    #p = subprocess.Popen(hkeygen, stdout = subprocess.PIPE, shell = True)
    #(output, err) = p.communicate()
    #server_prikey = output.split()[1]
    #server_pubkey = output.split()[3] 
    #print "Server's public key: ", server_pubkey
   
    pubkey_production_mode = "BIHqhi/x96ZLLFhDH7/4QPLYn/KTj1i1wRdQ7jcDIPknVs6E+ILJdZpw8yX2j+QhFKnJnnJDOiesCDEK2qVr7Rw="
    pubkey_testing_mode = "BLtIrjcmxXNzRKVLNGP+xJnLEIp9EboTe6PH0EO9bX4UmU9gRio/kVUHSbsq5UEfIrf5vueZVqRjwwitUI81V98="

    server_pubkey = pubkey_production_mode

    #query_hex = raw_input("Input the query data in bytes array: ")
    print quote(query)
    #query_b64 = base64.b64encode(query_hex.decode("hex"))
    #print "Encoded query: ", len(query_b64), query_b64
    
    henc = "{}henc {} {}".format(libdir, server_pubkey, quote(query))
    p = subprocess.Popen(henc, stdout = subprocess.PIPE, shell = True)
    (cipher_b64, err) = p.communicate()
    cipher_b64 = cipher_b64.split()[0]
    print "Encrypted query in base64: ", len(cipher_b64), cipher_b64
    
    #cipher_hex = cipher_b64.decode("base64").encode("hex")
    #cipher_hex = cipher_hex.split()[0]
    #print "Encrypted query in hex: ", len(cipher_hex), cipher_hex
    
    query_enc = [cipher_b64[i:i+32] for i in range(0, len(cipher_b64), 32)]
    print "Encrypted query in bytes32 array: ", query_enc
    
    #hdec = "{}hdec {} {}".format(libdir, server_prikey, cipher_b64)
    #p = subprocess.Popen(hdec, stdout = subprocess.PIPE, shell = True)
    #(plaintext, err) = p.communicate()
    #plaintext = plaintext.strip()
    #p_status = p.wait()
    #print "Command output: ", plaintext 
    #print "Command exit status/return code: ", p_status

    return cipher_b64


app = Flask(__name__)

@app.route('/query',methods = ['POST', 'GET'])
def query():
    if request.method == 'POST':
        query.plaintext = json.dumps({"flight": str(request.form.get('flight')), "time": long(request.form.get('time'))})
        print query.plaintext
        query.ciphertext = encQuery(query.plaintext)
        return render_template("query.html", query = query)

@app.route('/')
def index():
    return render_template('input.html')

if __name__ == '__main__':
    app.run('0.0.0.0')

