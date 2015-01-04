import unittest
from smartsnmp_testcase import *

class SNMPv3TestCase(unittest.TestCase, SmartSNMPTestCmd, SmartSNMPCommonCase):
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
		if self.snmp.isalive() == False:
			self.snmp.read()
			raise Exception("SNMP daemon start error!")

	def tearDown(self):
		if self.snmp.isalive() == False:
			self.snmp.read()
			raise Exception("SNMP daemon start error!")
		self.snmp_teardown()

	def test_snmpv3set(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess())
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType())
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888))
		self.snmpset_expect(".1.3.6.1.2.1.4.0", Integer(8888), SNMPNotWritable())

if __name__ == '__main__':
    unittest.main()
