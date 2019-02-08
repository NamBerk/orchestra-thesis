##### REQUIREMENTS ######

- InstantContiki3.0 (https://sourceforge.net/projects/contiki/files/Instant%20Contiki/Instant%20Contiki%203.0/InstantContiki3.0.zip/download)
- Oracle VM VirtualBox or VMware Player

##### Installation ######

1. start InstantContiki3.0
2. overwrite ~/contiki/ with RNC_JanSturm/contiki
3. start cooja: 
	cd contiki/tools/cooja/
	ant run
	(if error message : run "git submodule update --init"
4. open a simulation:
	1. File -> Open simulation -> Browse 
	2. Choose one of the simulations from RNC_JanSturm/simulations
	3. click on "Start" in the Simulation control

5. Parameters can be adjusted in 
	- ~/contiki/core/net/seemoo-lab/rnc/params.h
	- ~/contiki/core/net/seemoo-lab/flooding/params.h
