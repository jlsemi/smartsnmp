import unittest
import pexpect
from pprint import pprint
import re

logfile = open("tests/test.log", 'w')
env = {
	'LUA_PATH': "lualib/?/init.lua;lualib/?.lua;./?.lua",
	'LUA_CPATH': "build/?.so",
}

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

class SNMPTestBase(unittest.TestCase):
	@classmethod
	def setUpClass(self):
		self.agent = pexpect.spawn('./bin/smartsnmpd -c config/snmp.conf', logfile = logfile, env = env)
		self.version = "2c"
		self.community = "public"
		self.user = "noAuthUser"
		self.level = "noAuthNoPriv"
		self.ip = "127.0.0.1"
		self.port = 161

	@classmethod
	def tearDownClass(self):
		self.agent.close(force = True)

	def snmp_request(self, req, oids = [], tag = None, value = None, version = None, community = None, user = None, level = None, ip = None, port = None):
		# parse oid
		if isinstance(oids, str):
			oids = [oids]
		# snmp parameter initialize
		community = community or self.community
		user = user or self.user
		level = level or self.level
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
			if req == "set":
				snmp_req = "snmp%s -On -v%s -c%s %s:%d %s %s %r" % (req, version, community, ip, port, oid_str, tag[0].lower(), value)
			else:
				snmp_req = "snmp%s -On -v%s -c%s %s:%d %s" % (req, version, community, ip, port, oid_str)
		elif version == "3":
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
		normal_pattern = r"(?P<oid>\S+) = (?P<tag>[^:\r\n]+): [\"]*(?P<value>[^\r\n]+)[\"]*\r\n"
		not_found_pattern = r"(?P<oid>\S+) = (?P<error>[^\r\n]+)\r\n"
		error_status_pattern = r"Error in packet.\r\nReason: (?P<error>[^\r\n]+)\r\nFailed object: (?P<oid>[^\r\n]+)\r\n\r\n"
		timeout_pattern = r"Timeout: (?P<error>[^\r\n]+)\r\n"

		results = []
		while True:
			i = client.expect([pexpect.EOF, normal_pattern, not_found_pattern, error_status_pattern, timeout_pattern])
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

	def compare_get_result(self, result, oid, expect):
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

	def compare_set_result(self, result, oid, expect):
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

	def snmpget_expect(self, oid, expect, **kwargs):
		results = self.snmpget(oid, **kwargs)
		print results[0]
		self.compare_get_result(results[0], oid, expect)

	def snmpget_expects(self, args, **kwargs):
		oids = []
		for i in range(len(args)):
			oids += [args[i][0]]
		results = self.snmpget(oids, **kwargs)
		for i in range(len(results)):
			print results[i]
			self.compare_get_result(results[i], oids[i], args[i][1])

	def snmpgetnext_expect(self, req_oid, ret_oid, expect, **kwargs):
		results = self.snmpgetnext(req_oid, **kwargs)
		print results[0]
		self.compare_get_result(results[0], ret_oid, expect)

	def snmpset_expect(self, oid, setting, expect, **kwargs):
		results = self.snmpset(oid, setting, **kwargs)
		print(results[0])
		self.compare_set_result(results[0], oid, expect)


class SNMPv2cTestCase(SNMPTestBase):
	def test_get(self):
		self.snmpget_expect(".1.3.6.1.2.1.7.5.1.2.4", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.2.1.0", Integer(5))
		self.snmpget_expect(".", SNMPNoSuchObject())
		self.snmpget_expect(".0", SNMPNoSuchObject())
		self.snmpget_expect(".1.3", SNMPNoSuchObject())
		self.snmpget_expect(".1.4", SNMPNoSuchObject())
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2.1", Oid(".1.3.6.1.2.1.4"))
		self.snmpget_expects(((".1.3.6.1.2.1.1.9.1.0", SNMPNoSuchObject()), (".1.3.6.1.2.1.1.2.0", Oid(".1.3.6.1.2.1.1"))))
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.1", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.5", SNMPNoSuchObject())

	def test_getnext(self):
		self.snmpgetnext_expect(".", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"))
		self.snmpgetnext_expect(".0", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"))
		self.snmpgetnext_expect(".1.3", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"))
		self.snmpgetnext_expect(".1.4", ".1.4", SNMPEndOfMib())
		self.snmpgetnext_expect(".1.5.6.7.8.100", ".1.5.6.7.8.100", SNMPEndOfMib())

	def test_set(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPAuthErr(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPAuthErr(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), SNMPAuthErr(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess(), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType(), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888), community = "private")

	def test_walk(self):
		self.snmp_request('walk', ['.'])


class SNMPv3TestCase(SNMPTestBase):
	def test_get(self):
		self.snmpget_expect(".1.3.6.1.2.1.7.5.1.2.4", SNMPNoSuchInstance(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.2.1.0", Integer(5), version = '3')
		self.snmpget_expect(".", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".0", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.3", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.4", SNMPNoSuchObject(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2.1", Oid(".1.3.6.1.2.1.4"), version = '3')
		self.snmpget_expects(((".1.3.6.1.2.1.1.9.1.0", SNMPNoSuchObject()), (".1.3.6.1.2.1.1.2.0", Oid(".1.3.6.1.2.1.1"))), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.1", SNMPNoSuchInstance(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2", SNMPNoSuchInstance(), version = '3')
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.5", SNMPNoSuchObject(), version = '3')

	def test_getnext(self):
		self.snmpgetnext_expect(".", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"), version = '3')
		self.snmpgetnext_expect(".0", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"), version = '3')
		self.snmpgetnext_expect(".1.3", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"), version = '3')
		self.snmpgetnext_expect(".1.4", ".1.4", SNMPEndOfMib(), version = '3')
		self.snmpgetnext_expect(".1.5.6.7.8.100", ".1.5.6.7.8.100", SNMPEndOfMib(), version = '3')

	def test_set(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess(), version = '3')
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType(), version = '3')
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888), version = '3')

	def test_walk(self):
		self.snmp_request('walk', ['.'], version = '3')


if __name__ == '__main__':
	unittest.main()
