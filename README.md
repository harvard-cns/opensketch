opensketch
==========
We release our simulator(simulation/) and prototype(controller/ and netfpga/) code here.

simulation:
------------------
This is a packet-trace(CAIDA packet trace) driven simulator we built to compare with NetFlow 
and other streaming algorithms. From our evalution, it provides a better memory-accuracy tradeoff
than NetFlow and achieves comparable accuracy to the streaming algorithms, while provindg 
generality. This OpenSketch simulation platform has heavy hitters and superspreaders 
implementation as examples. Many other measurement tasks can be implemented on top of it with 
simple controller code and limited data plane resources, because it only need to configure existing
building blocks and analyze the collected data to report the results. We have demonstrated five
measurement task, and each needs only 10-25 lines of code for configuration and 150 lines of code 
for analysis.


controller: 
------------------
This is control plane code for our prototype. We implemented heavy hitters and superspreaders 
as examples. Each sketch has two functions for the measurement program to use:

(1)configure : to specify the packet fields, the memory constraint, and the number of hash functions to use
            
(2)query     : to periodically get the statistics

This two basic functions also defines two types of APIs, including set Hash function, constrain sram counter
size, and dump sram counter, etc. The controller employs PCI interface to communicate with data plane, which
reads or writes the hardware registers through IOCTL calls.

We also implement the sketch manager to conﬁgure these sketches and the resource allocator to
divide resources across multiple measurement tasks.
The measurement program in the controller can periodically query the sketches about the statistics. 
According to the received statistics, the measurement programs may install new sketches or change
the accuracy requirements accordingly. The sketches automatically queries the data plane to get the counters
in order to generate the right statistics to the measurement program. 

netfpga: 
------------------
This is data plane prototye in netfpga, including Header Parser, Hashing, Wildcard Lookup, and
SRAM Counter in a pipeline. Since we simply pull packet headers to collect the statistics without changing the packet
bus, there is no effect on packet forwarding latency and throughput. As a packet enters, the Header Parser pulls
the related ﬁelds from the packet header, which are then
hashed by hash functions in parallel. We then pass the
packet header and hash values to the Wildcard Lookup,
where we implement wildcard rule matching in parallel. For each matched rule, we update the corresponding
counters in the SRAM Counter. We use the entire 4.5
MB on-board SRAM to store the counters.

This OpenSketch modules should be inserted into the reference switch pipeline and then compiled following
NetFPGA user guide. We implemented heavy hitters and superspreaders
as examples. We plan to implement more sketch functions in the future.

--------------------------
We are willing to hear your feedback. If you have further questions, feel free to contact us. 
rmiao@usc.edu lavanyaj@cs.stanford.edu

