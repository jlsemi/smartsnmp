import unittest
from smartsnmp_testcase import *

class AgentXv3TestCase(unittest.TestCase, SmartSNMPTestCmd, SmartSNMPCommonCase):
	def setUp(self):
		self.agentx_setup("config/agentx.conf")
		self.version = "3"
		self.user = "authOnlyUser"
		self.level = "authNoPriv"
		self.auth_protocol = "MD5"
		self.auth_key = "authOnlyUser"
		self.priv_protocol = ""
		self.priv_key = ""
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

	def test_snmpv3set(self):
		self.snmpset_expect(".1.3.6.1.2.1.1.9.1.1", Integer(1), SNMPNoAccess())
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", OctStr("SmartSNMP"), SNMPWrongType())
		self.snmpset_expect(".1.3.6.1.2.1.4.1.0", Integer(8888), Integer(8888))
		self.snmpset_expect(".1.3.6.1.2.1.4.0", Integer(8888), SNMPNotWritable())

if __name__ == '__main__':
    unittest.main()
