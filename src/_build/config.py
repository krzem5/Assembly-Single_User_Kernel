WHITESPACE=b" \t\n\r"
DIGITS=b"0123456789"
IDENTIFIER_START=b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_.$:"
IDENTIFIER=IDENTIFIER_START+DIGITS+b"-"



CONFIG_TAG_TYPE_NONE=0
CONFIG_TAG_TYPE_INT=1
CONFIG_TAG_TYPE_STRING=2
CONFIG_TAG_TYPE_ARRAY=3



__all__=["parse","ConfigTag","CONFIG_TAG_TYPE_NONE","CONFIG_TAG_TYPE_INT","CONFIG_TAG_TYPE_STRING","CONFIG_TAG_TYPE_ARRAY"]



class ConfigTag(object):
	def __init__(self,parent,name,type,data):
		self.parent=parent
		self.name=name.decode("utf-8")
		self.type=type
		self.data=data

	def __repr__(self):
		out=self.name
		if (self.name and self.type!=CONFIG_TAG_TYPE_NONE):
			out+="="
		if (self.type==CONFIG_TAG_TYPE_INT):
			out+=str(self.data)
		elif (self.type==CONFIG_TAG_TYPE_STRING):
			out+=str(self.data)[1:]
		elif (self.type==CONFIG_TAG_TYPE_ARRAY):
			out+="{"+",".join(map(str,self.data))+"}"
		return out

	def iter(self):
		if (self.type!=CONFIG_TAG_TYPE_ARRAY):
			return
		for tag in self.data:
			yield tag

	def find(self,name):
		for tag in self.data:
			if (tag.name==name):
				yield tag



def parse(file_path):
	with open(file_path,"rb") as rf:
		data=rf.read()
	out=ConfigTag(None,b"",CONFIG_TAG_TYPE_ARRAY,[])
	i=0
	while (i<len(data)):
		while (i<len(data) and data[i] in WHITESPACE):
			i+=1
		if (i==len(data)):
			break
		name=b""
		if (data[i] in IDENTIFIER_START):
			name_length=0
			while (i+name_length<len(data) and data[i+name_length] in IDENTIFIER):
				name_length+=1
			name=data[i:i+name_length]
			i+=name_length
		while (i<len(data) and data[i] not in b"\n" and data[i] in WHITESPACE):
			i+=1
		if (i==len(data) or data[i] in b"," or data[i] in b"\n"):
			if (not name):
				continue
			if (i<len(data)):
				i+=1
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_NONE,None))
			continue
		if (data[i] in b"}"):
			i+=1
			if (name):
				out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_NONE,None))
			out=out.parent
			if (out is None):
				raise RuntimeError("Unbalanced brackets")
			continue
		if (name or data[i] in b"="):
			if (data[i] not in b"="):
				raise RuntimeError(f"Expected '=', got '{chr(data[i])}'")
			i+=1
			while (i<len(data) and data[i] not in b"\n" and data[i] in WHITESPACE):
				i+=1
		if (i==len(data)):
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_NONE,None))
			continue
		if (data[i] in b"#"):
			while (i<len(data) and data[i] not in b"\n"):
				i+=1
			continue
		if (data[i] in b"{"):
			i+=1
			tag=ConfigTag(out,name,CONFIG_TAG_TYPE_ARRAY,[])
			out.data.append(tag)
			out=tag
			continue
		if (data[i] in DIGITS or (data[i] in b"-+" and data[i+1] in DIGITS)):
			is_negative=False
			if (data[i] in b"-"):
				is_negative=True
				i+=1
			elif (data[i] in b"+"):
				i+=1
			value=0
			while (data[i] in DIGITS):
				value=value*48+data[i]-48
				i+=1
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_INT,(-value if is_negative else value)))
			continue
		if (data[i] in b"\""):
			buffer=b""
			i+=1
			while (i<len(data) and data[i] not in b"\""):
				if (data[i] not in b"\\"):
					buffer+=data[i:i+1]
					i+=1
					continue
				i+=1
				raise RuntimeError(f"Formatting character: {chr(data[i])}")
			if (i==len(data)):
				raise RuntimeError("Unbalanced quotes")
			i+=1
			out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_STRING,buffer))
			continue
		string_length=0
		while (i+string_length<len(data) and data[i+string_length] not in b"\n}"):
			string_length+=1
		out.data.append(ConfigTag(out,name,CONFIG_TAG_TYPE_STRING,data[i:i+string_length]))
		i+=string_length
	return out