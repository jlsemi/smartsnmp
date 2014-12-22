import pexpect, sys, re, time
from pprint import pprint
from functools import wraps

logfile = open("tests/test.log", 'w')
env = {
	'LD_LIBRARY_PATH': ".",
	'LUA_PATH': "lualib/?/init.lua;lualib/?.lua;./?.lua",
	'LUA_CPATH': "build/?.so",
}

# ASN.1 tag
class SNMPASN1Tag:
	def __init__(self, value):
		self.value = str(value)
class Integer(SNMPASN1Tag): tag = "INTEGER"
class OctStr(SNMPASN1Tag): tag = "STRING"
class Timeticks(SNMPASN1Tag): tag = "Timeticks"
class HexStr(SNMPASN1Tag): tag = "Hex-STRING"
class Oid(SNMPASN1Tag):
	def __init__(self, value):
		SNMPASN1Tag.__init__(self, value)
		if self.value[0] != '.':
			self.value = '.' + self.value
	tag = "OID"
class IpAddress(SNMPASN1Tag): tag = "IpAddress"
class Count32(SNMPASN1Tag): tag = "Count32"
class Gauge32(SNMPASN1Tag): tag = "Gauge32"

# Error status
class SNMPErrorStatus: pass 
class SNMPNoSuchObject(SNMPErrorStatus): value = "No Such Object available on this agent at this OID"
class SNMPNoSuchInstance(SNMPErrorStatus): value = "No Such Instance currently exists at this OID"
class SNMPEndOfMib(SNMPErrorStatus): value = "No more variables left in this MIB View (It is past the end of the MIB tree)"
class SNMPTooBig(SNMPErrorStatus): value = "(tooBig) Response message would have been too large."
class SNMPNoSuchName(SNMPErrorStatus): value = "(noSuchName) There is no such variable name in this MIB."
class SNMPBadValue(SNMPErrorStatus): value = "(badValue) The value given has the wrong type or length."
class SNMPReadOnly(SNMPErrorStatus): value = "(readOnly) The two parties used do not have access to use the specified SNMP PDU."
class SNMPGenErr(SNMPErrorStatus): value = "(genError) A general failure occured"
class SNMPNoAccess(SNMPErrorStatus): value = "noAccess"
class SNMPWrongType(SNMPErrorStatus): value = "wrongType (The set datatype does not match the data type the agent expects)"
class SNMPWrongLength(SNMPErrorStatus): value = "wrongLength (The set value has an illegal length from what the agent expects)"
class SNMPWrongEncoding(SNMPErrorStatus): value = "wrongEncoding"
class SNMPWrongValue(SNMPErrorStatus): value = "wrongValue (The set value is illegal or unsupported in some way)"
class SNMPNoCreation(SNMPErrorStatus): value = "noCreation (That table does not support row creation or that object can not ever be created)"
class SNMPInconsistValue(SNMPErrorStatus): value = "inconsistentValue (The set value is illegal or unsupported in some way)"
class SNMPResourceUna(SNMPErrorStatus): value = "resourceUnavailable (This is likely a out-of-memory failure within the agent)"
class SNMPCommitFail(SNMPErrorStatus): value = "commitFailed"
class SNMPUndoFail(SNMPErrorStatus): value = "undoFailed"
class SNMPAuthErr(SNMPErrorStatus): value = "authorizationError (access denied to that object)"
class SNMPNoWritable(SNMPErrorStatus): value = "notWritable (That object does not support modification)"

class SmartSNMPTestCmd:
	def snmp_request(self, req, oids = [], tag = None, value = None, version = None, community = None, user = None, level = None, ip = None, port = None):
		# parse oid
		if isinstance(oids, str):
			oids = [oids]
		# snmp parameter initialize
		version = version or self.version
		ip = ip or self.ip
		port = port or self.port
		# generate snmp request oid
		oid_list = []
		for oid in oids:
			if isinstance(oid, tuple):
				oid_list += list(oid)
			elif isinstance(oid, str):
				oid_list += [oid]
		oid_str = " ".join(oid_list)

		if version == "1" or version == "2c":
			community = community or self.community
			if req == "set":
				snmp_req = "snmp%s -On -v%s -c%s %s:%d %s %s %r" % (req, version, community, ip, port, oid_str, tag[0].lower(), value)
			else:
				snmp_req = "snmp%s -On -v%s -c%s %s:%d %s" % (req, version, community, ip, port, oid_str)
		elif version == "3":
			user = user or self.user
			level = level or self.level
			if req == "set":
				snmp_req = "snmp%s -On -v%s -u%s -l%s %s:%d %s %s %r" % (req, version, user, level, ip, port, oid_str, tag[0].lower(), value)
			else:
				snmp_req = "snmp%s -On -v%s -u%s -l%s %s:%d %s" % (req, version, user, level, ip, port, oid_str)
		else:
			snmp_req = ""
		print(snmp_req)

		# create client
		client = pexpect.spawn(snmp_req, logfile = logfile, env = env)

		# match pattern for snmpreq output
		normal_pattern = r"(?P<oid>\S+) = (?P<tag>[^:\r\n]+): [\"]*(?P<value>[^\r\n]*)[\"]*\r\n"
		empty_value_pattern = r"(?P<oid>\S+) = (?P<value>\"\")\r\n"
		not_found_pattern = r"(?P<oid>\S+) = (?P<error>[^\r\n]+)\r\n"
		error_status_pattern = r"Error in packet.\r\nReason: (?P<error>[^\r\n]+)\r\nFailed object: (?P<oid>[^\r\n]+)\r\n\r\n"
		timeout_pattern = r"No response from (?P<ip>[^:]+):(?P<port>[^\r\n]+)\r\n"

		results = []
		while True:
			i = client.expect([pexpect.EOF, normal_pattern, empty_value_pattern, not_found_pattern, error_status_pattern, timeout_pattern])
			if i == 0:
				break
			else:
				results.append(client.match.groupdict())
		return results

	def snmpget(self, oids, **kwargs):
		return self.snmp_request('get', oids, **kwargs)

	def snmpgetnext(self, oids, **kwargs):
		return self.snmp_request('getnext', oids, **kwargs)

	def snmpset(self, oids, setting, **kwargs):
		return self.snmp_request('set', oids, setting.tag, setting.value, **kwargs)

	def snmpwalk(self, oid, **kwargs):
		return self.snmp_request('walk', oid, **kwargs)

	def snmpget_result_check(self, result, oid, expect):
		# return OID match
		if oid == '.':
			assert(result["oid"] == ".0.1")
		elif str.isdigit(oid):
			assert(result["oid"] == ".0." + oid)
		elif oid[0] == '.' and str.isdigit(oid[1:]):
			assert(result["oid"] == ".0." + oid[1:])
		else:
			assert(result["oid"] == oid)
		# value
		assert(isinstance(expect, SNMPASN1Tag) or isinstance(expect, SNMPErrorStatus))
		if isinstance(expect, SNMPASN1Tag):
			assert(expect.tag == result["tag"])
			assert(expect.value == result["value"] or re.match(expect.value, result["value"]))
		else:
			assert(expect.value == result["error"] or re.match(expect.value, result["error"]))

	def snmpset_result_check(self, result, oid, expect):
		# result
		assert(isinstance(expect, SNMPASN1Tag) or isinstance(expect, SNMPErrorStatus))
		if isinstance(expect, SNMPASN1Tag):
			# return OID match
			if oid == '.':
				assert(result["oid"] == ".0.1")
			elif str.isdigit(oid):
				assert(result["oid"] == ".0." + oid)
			elif oid[0] == '.' and str.isdigit(oid[1:]):
				assert(result["oid"] == ".0." + oid[1:])
			else:
				assert(result["oid"] == oid)
			# tag
			assert(expect.tag == result["tag"])
			assert(expect.value == result["value"] or re.match(expect.value, result["value"]))
		else:
			# error message
			assert(expect.value == result["error"] or re.match(expect.value, result["error"]))
			# oid object
			assert(oid == result["oid"])

	def snmpwalk_result_check(self, result, length, index):
		if index != length - 1:
			assert(result.has_key("oid") and result.has_key("value"))
		else:
			assert(result.has_key("oid") and result.has_key("error") and result["error"] == SNMPEndOfMib().value)

	def snmpget_expect(self, oid, expect, **kwargs):
		results = self.snmpget(oid, **kwargs)
		print results[0]
		self.snmpget_result_check(results[0], oid, expect)

	def snmpget_expects(self, args, **kwargs):
		oids = []
		for i in range(len(args)):
			oids += [args[i][0]]
		results = self.snmpget(oids, **kwargs)
		for i in range(len(results)):
			print results[i]
			self.snmpget_result_check(results[i], oids[i], args[i][1])

	def snmpgetnext_expect(self, req_oid, ret_oid, expect, **kwargs):
		results = self.snmpgetnext(req_oid, **kwargs)
		print results[0]
		self.snmpget_result_check(results[0], ret_oid, expect)

	def snmpset_expect(self, oid, setting, expect, **kwargs):
		results = self.snmpset(oid, setting, **kwargs)
		print(results[0])
		self.snmpset_result_check(results[0], oid, expect)

	def snmpwalk_expect(self, oid, **kwargs):
		results = self.snmpwalk(oid, **kwargs)
		for i in range(len(results)):
			print(results[i])
			self.snmpwalk_result_check(results[i], len(results), i)

	def snmp_setup(self, config_file):
		self.snmp = pexpect.spawn(r"./bin/smartsnmpd -c " + config_file, env = env)
		self.snmp.logfile_read = sys.stderr
		time.sleep(1)

	def snmp_teardown(self):
		self.snmp.close(force = True)

	def agentx_setup(self, config_file):
		self.netsnmp = pexpect.spawn(r"./tests/net-snmp-release/sbin/snmpd -f -Lo -m "" -C -c tests/snmpd.conf")
		time.sleep(1)
		self.agentx = pexpect.spawn(r"./bin/smartsnmpd -c " + config_file, env = env)
		self.agentx.logfile_read = sys.stderr
		time.sleep(1)

	def agentx_teardown(self):
		self.agentx.close(force = True)
		self.netsnmp.close(force = True)

def snmp_before_check(test_func):
	@wraps(test_func)
	def wrapper(daemon):
		if daemon.snmp.isalive() == False:
			daemon.snmp.read()
			raise Exception("SNMP daemon start error!")
		else:
			test_func(daemon)
	return wrapper

def agentx_before_check(test_func):
	@wraps(test_func)
	def wrapper(daemon):
		if daemon.agentx.isalive() == False:
			daemon.agentx.read()
			raise Exception("AgentX daemon start error!")
		else:
			test_func(daemon)
	return wrapper
