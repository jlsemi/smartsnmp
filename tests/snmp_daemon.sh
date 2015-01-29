if [ -n "$LUACOV" ]; then
	export SYS_LUA_PATH=`lua -e "print(package.path)"`;
	export LUACOV_OPT=-lluacov
fi
LUA_PATH="lualib/?/init.lua;lualib/?.lua;./?.lua;$SYS_LUA_PATH" LUA_CPATH="build/?.so" lua $LUACOV_OPT ./bin/smartsnmpd -c config/snmp.conf
