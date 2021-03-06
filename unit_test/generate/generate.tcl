#Author: Juexiao Su
#Purpose: Test basic functionality

puts "#########################################"
puts "#        read blif netlist              #"
puts "#########################################"
set design 3gate.blif
read_blif $design
gen_dwave_nl
puts "\n"

puts "#########################################"
puts "#     initialize hardware target        #"
puts "#########################################"
init_target -row 16 -col 16 -local 8
write_blif 3gate_dwave.blif
puts "\n"

puts "#########################################"
puts "#     initialize place and route        #"
puts "#########################################"
init_system 
puts "\n"

puts "#########################################"
puts "#           place netlist               #"
puts "#########################################"
place
puts "\n"


#puts "#########################################"
#puts "#     check routing graph               #"
#puts "#########################################"
#check_routing_graph
#puts "\n"


puts "#########################################"
puts "#         route netlsit                 #"
puts "#########################################"
route
puts "\n"


puts "#########################################"
puts "#        generate config                #"
puts "#########################################"
generate
puts "\n"



