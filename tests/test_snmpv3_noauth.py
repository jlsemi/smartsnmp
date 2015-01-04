import unittest
from smartsnmp_testcmd import *

class SNMPv3TestCase(unittest.TestCase, SmartSNMPTestCmd):
	def setUp(self):
		self.snmp_setup("config/snmp.conf")
		self.version = "3"
		self.user = "noAuthUser"
		self.level = "noAuthNoPriv"
		self.auth_protocol = ""
		self.auth_key = ""
		self.priv_protocol = ""
		self.priv_key = ""
		self.ip = "127.0.0.1"
		self.port = 161

	def tearDown(self):
		self.snmp_teardown()

	@snmp_before_check
	def test_snmpv3get(self):
		self.snmpget_expect(".", SNMPNoSuchObject())
		self.snmpget_expect(".0", SNMPNoSuchObject())
		self.snmpget_expect(".1.3", SNMPNoSuchObject())
		self.snmpget_expect(".1.4", SNMPNoSuchObject())
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2.1", Oid(".1.3.6.1.2.1.4"))
		self.snmpget_expects(((".1.3.6.1.2.1.1.9.1.0", SNMPNoSuchObject()), (".1.3.6.1.2.1.1.2.0", Oid(".1.3.6.1.2.1.1"))))
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.1", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.2", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.1.9.1.5", SNMPNoSuchObject())
		self.snmpget_expect(".1.3.6.1.2.1.1.0", SNMPNoSuchObject())

		# a correct request for MIB-II/UDP should be .1.3.6.1.2.1.7.5.1.2.4.x.x.x.x.p
		self.snmpget_expect(".1.3.6.1.2.1.7.5.1.2.4", SNMPNoSuchInstance())
		self.snmpget_expect(".1.3.6.1.2.1.7.5.1.2.4.1.1", SNMPNoSuchInstance())

	@snmp_before_check
	def test_snmpv3getnext(self):
		self.snmpgetnext_expect(".", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"))
		self.snmpgetnext_expect(".0", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"))
		self.snmpgetnext_expect(".1.3", ".1.3.6.1.2.1.1.1.0", OctStr(r".*"))
		self.snmpgetnext_expect(".1.4", ".1.4", SNMPEndOfMib())
		self.snmpgetnext_expect(".1.5.6.7.8.100", ".1.5.6.7.8.100", SNMPEndOfMib())

	@snmp_before_check
	def test_snmpv3set(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess())
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType())
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888))
		self.snmpset_expect(".1.3.6.1.2.1.4.0", Integer(8888), SNMPNotWritable())

	@snmp_before_check
	def test_snmpv3walk(self):
		self.snmpwalk_expect(".")


if __name__ == '__main__':
    unittest.main()
