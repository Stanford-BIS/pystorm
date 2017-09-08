------------------------------------------------------------------------
--okLibrary.vhd 
--
--FrontPanel Library Module Declarations (VHDL)
-- ZEM5305
--
-- Copyright (c) 2004-2011 Opal Kelly Incorporated
-- $Rev: 10 $ $Date: 2015-01-27 17:01:53 -0800 (Tue, 27 Jan 2015) $
------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity okHost is
	port (
		okUH   : in    std_logic_vector(4 downto 0);
		okHU   : out   std_logic_vector(2 downto 0);
		okUHU  : inout std_logic_vector(31 downto 0);
		okAA   : inout std_logic;
		okClk  : out   std_logic;
		okHE   : out   std_logic_vector(112 downto 0);
		okEH   : in    std_logic_vector(64 downto 0)
	);
end okHost;

architecture archHost of okHost is
	
	component okCoreHarness port (
		okHC   : in    std_logic_vector(38 downto 0);
		okCH   : out   std_logic_vector(37 downto 0);
		okHE   : out   std_logic_vector(112 downto 0);
		okEH   : in    std_logic_vector(64 downto 0));
	end component;
	
	component ok_altera_pll generic (
		phase    : string
		);
		port (
		refclk   : in    std_logic;
		rst      : in    std_logic;
		outclk_0 : out   std_logic;
		locked   : out   std_logic);
	end component;
	
	signal okHC             : std_logic_vector(38 downto 0);
	signal okCH             : std_logic_vector(37 downto 0);
	
	signal data_valid       : std_logic;
	signal data_out         : std_logic_vector(31 downto 0);
	signal data_in          : std_logic_vector(31 downto 0);
	signal ctrl_in          : std_logic_vector(4 downto 1);
	
	signal pll_locked       : std_logic;
	signal notCH36          : std_logic;
	
	
begin
	
	okHC(38)          <= not(pll_locked);
	okHC(37)          <= okAA;
	okHC(36 downto 5) <= data_in;
	okHC(4 downto 1)  <= ctrl_in;
	okClk             <= okHC(0);
	
	--Clock	
	ok_altera_pll0: ok_altera_pll generic map (
		phase => "-2014 ps"
	) port map (
		refclk    => okUH(0),
		rst       => '0',
		outclk_0  => okHC(0),
		locked    => pll_locked
	);
	
	------------------------------------------------------------------------
	-- Bidirectional IOB registers
	------------------------------------------------------------------------
		--Tristate
		okUHU <= data_out WHEN (data_valid = '1') ELSE (others => 'Z');
		
		-- Input Registering
		process (okHC(0)) begin
			if rising_edge(okHC(0)) then
				data_in <= okUHU;
			end if; 
		end process;
		
		-- Output Registering
		process (okHC(0)) begin
			if rising_edge(okHC(0)) then
				data_out <= okCH(34 downto 3);
			end if; 
		end process;
			
		-- Tristate Drive
		process (okHC(0)) begin
			if rising_edge(okHC(0)) then
				data_valid <= okCH(36);
			end if; 
		end process;
	
	okAA <= okCH(35) WHEN (okCH(37) = '0') ELSE 'Z';

	------------------------------------------------------------------------
	-- Output IOB registers
	------------------------------------------------------------------------
	process (okHC(0)) begin
		if rising_edge(okHC(0)) then
			okHU(2) <= okCH(2);
			okHU(0) <= okCH(0);
			okHU(1) <= okCH(1);
		end if; 
	end process;

	--------------------------------------------------------------------------
	-- Input IOB registers
	--  - First registered on DCM0 (positive edge)
	--  - Then registered on DCM0 (negative edge)
	--------------------------------------------------------------------------
	process (okHC(0)) begin
		if rising_edge(okHC(0)) then
			ctrl_in(1)   <= okUH(1);
			ctrl_in(4 downto 2) <= okUH(4 downto 2);
		end if; 
	end process;

	core0 : okCoreHarness port map(okHC=>okHC, okCH=>okCH, okHE=>okHE, okEH=>okEH);
end archHost;

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
entity okWireOR is
	generic (
		N     : integer := 1
	);
	port (
		okEH   : out std_logic_vector(64 downto 0);
		okEHx  : in  std_logic_vector(N*65-1 downto 0)
	);
end okWireOR;
architecture archWireOR of okWireOR is
begin
	process (okEHx)
		variable okEH_int : STD_LOGIC_VECTOR(64 downto 0);
	begin
		okEH_int:= '0' & x"0000_0000_0000_0000";
		for i in N-1 downto 0 loop
			okEH_int := okEH_int or okEHx( (i*65+64) downto (i*65) );
		end loop;
		okEH <= okEH_int;
	end process;
end archWireOR;

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
package FRONTPANEL is
	
	component okHost port (
		okUH   : in    std_logic_vector(4 downto 0);
		okHU   : out   std_logic_vector(2 downto 0);
		okUHU  : inout std_logic_vector(31 downto 0);
		okAA   : inout std_logic;
		okClk  : out   std_logic;
		okHE   : out   std_logic_vector(112 downto 0);
		okEH   : in    std_logic_vector(64 downto 0));
	end component;

	component okCoreHarness port (
		okHC   : in    std_logic_vector(38 downto 0);
		okCH   : out   std_logic_vector(37 downto 0);
		okHE   : out   std_logic_vector(112 downto 0);
		okEH   : in    std_logic_vector(64 downto 0));
	end component;

	component okWireIn port (
		okHE       : in  std_logic_vector(112 downto 0);
		ep_addr    : in  std_logic_vector(7 downto 0);
		ep_dataout : out std_logic_vector(31 downto 0));
	end component;

	component okWireOut port (
		okHE       : in  std_logic_vector(112 downto 0);
		okEH       : out std_logic_vector(64 downto 0);
		ep_addr    : in  std_logic_vector(7 downto 0);
		ep_datain  : in  std_logic_vector(31 downto 0));
	end component;

	component okTriggerIn port (
		okHE       : in  std_logic_vector(112 downto 0);
		ep_addr    : in  std_logic_vector(7 downto 0);
		ep_clk     : in  std_logic;
		ep_trigger : out std_logic_vector(31 downto 0));
	end component;

	component okTriggerOut port (
		okHE       : in  std_logic_vector(112 downto 0);
		okEH       : out std_logic_vector(64 downto 0);
		ep_addr    : in  std_logic_vector(7 downto 0);
		ep_clk     : in  std_logic;
		ep_trigger : in  std_logic_vector(31 downto 0));
	end component;

	component okPipeIn port (
		okHE       : in  std_logic_vector(112 downto 0);
		okEH       : out std_logic_vector(64 downto 0);
		ep_addr    : in  std_logic_vector(7 downto 0);
		ep_write   : out std_logic;
		ep_dataout : out std_logic_vector(31 downto 0));
	end component;

	component okPipeOut port (
		okHE       : in  std_logic_vector(112 downto 0);
		okEH       : out std_logic_vector(64 downto 0);
		ep_addr    : in  std_logic_vector(7 downto 0);
		ep_read    : out std_logic;
		ep_datain  : in  std_logic_vector(31 downto 0));
	end component;

	component okBTPipeIn port (
		okHE           : in  std_logic_vector(112 downto 0);
		okEH           : out  std_logic_vector(64 downto 0);
		ep_addr        : in  std_logic_vector(7 downto 0);
		ep_write       : out std_logic;
		ep_blockstrobe : out std_logic;
		ep_dataout     : out std_logic_vector(31 downto 0);
		ep_ready       : in  std_logic);
	end component;

	component okBTPipeOut port (
		okHE           : in  std_logic_vector(112 downto 0);
		okEH           : out std_logic_vector(64 downto 0);
		ep_addr        : in  std_logic_vector(7 downto 0);
		ep_read        : out std_logic;
		ep_blockstrobe : out std_logic;
		ep_datain      : in  std_logic_vector(31 downto 0);
		ep_ready       : in  std_logic);
	end component;
	
	component okRegisterBridge port (
		okHE           : in  std_logic_vector(112 downto 0);
		okEH           : out std_logic_vector(64 downto 0);
		ep_address     : out std_logic_vector(31 downto 0);
		ep_write       : out std_logic;
		ep_dataout     : out std_logic_vector(31 downto 0);
		ep_read        : out std_logic;
		ep_datain      : in  std_logic_vector(31 downto 0));
	end component;

	component okWireOR
	generic (N : integer := 1);
	port (
		okEH   : out std_logic_vector(64 downto 0);
		okEHx  : in  std_logic_vector(N*65-1 downto 0));
	end component;

end FRONTPANEL;
