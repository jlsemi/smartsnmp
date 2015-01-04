import unittest
from smartsnmp_testcase import *

class AgentXv2cTestCase(unittest.TestCase, SmartSNMPTestCmd, SmartSNMPCommonCase):
	def setUp(self):
		self.agentx_setup("config/agentx.conf")
		self.version = "2c"
		self.community = "public"
		self.ip = "127.0.0.1"
		self.port = 161
		if self.netsnmp.isalive() == False:
			self.netsnmp.read()
			raise Exception("NetSNMP daemon start error!")
		if self.agentx.isalive() == False:
			self.agentx.read()
			raise Exception("AgentX daemon start error!")

	def tearDown(self):
		if self.netsnmp.isalive() == False:
			self.netsnmp.read()
			raise Exception("NetSNMP daemon start error!")
		if self.agentx.isalive() == False:
			self.agentx.read()
			raise Exception("AgentX daemon start error!")
		self.agentx_teardown()

	def test_snmpv2cset(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPNoAccess(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), SNMPNoAccess(), community = "public")
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess(), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType(), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(0x1234), Integer(0x1234), community = "private")
		self.snmpset_expect(".1.3.6.1.2.1.4.0", Integer(8888), SNMPNotWritable(), community = "private")

if __name__ == '__main__':
    unittest.main()
