import os
import subprocess
import sys



__all__=["load_key","get_public_key","sign"]



_signature_key=[0,0,0]



def _decode_hex(data):
	out=bytearray(len(data)>>1)
	for i,e in enumerate(data.lower()):
		out[i>>1]|=(e-(87 if e>=97 else 48))<<(4-4*(i&1))
	return out



def load_key(file_path):
	if (not os.path.exists(file_path)):
		if (subprocess.run(["openssl","genpkey","-algorithm","rsa","-pkeyopt","rsa_keygen_bits:4096","-out",file_path]).returncode):
			sys.exit(1)
	process=subprocess.run(["openssl","asn1parse","-in",file_path,"-inform","pem"],stdout=subprocess.PIPE)
	if (process.returncode):
		sys.exit(1)
	buffer=_decode_hex(process.stdout.split(b"[HEX DUMP]:")[1].strip())
	process=subprocess.run(["openssl","asn1parse","-inform","der"],stdout=subprocess.PIPE,input=buffer)
	if (process.returncode):
		sys.exit(1)
	buffer=process.stdout.split(b"\n")
	n=int.from_bytes(_decode_hex(buffer[2].split(b":")[-1]),"big")
	e=int.from_bytes(_decode_hex(buffer[3].split(b":")[-1]),"big")
	d=int.from_bytes(_decode_hex(buffer[4].split(b":")[-1]),"big")
	if (d==e or d>=n or e>=n or pow(0x12345,d*e,n)!=0x12345):
		print("Invalid RSA key")
		sys.exit(1)
	_signature_key[0]=d
	_signature_key[1]=e
	_signature_key[2]=n



def get_public_key():
	return (_signature_key[1],_signature_key[2])



def sign(digest):
	return pow(int.from_bytes(digest,"little"),_signature_key[0],_signature_key[2]).to_bytes(4096,"little")