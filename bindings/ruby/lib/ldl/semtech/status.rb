require 'json'

module LDL::Semtech

    class Status
    
=begin
         Name |  Type  | Function
        :----:|:------:|--------------------------------------------------------------
         time | string | UTC 'system' time of the gateway, ISO 8601 'expanded' format
         lati | number | GPS latitude of the gateway in degree (float, N is +)
         long | number | GPS latitude of the gateway in degree (float, E is +)
         alti | number | GPS altitude of the gateway in meter RX (integer)
         rxnb | number | Number of radio packets received (unsigned integer)
         rxok | number | Number of radio packets received with a valid PHY CRC
         rxfw | number | Number of radio packets forwarded (unsigned integer)
         ackr | number | Percentage of upstream datagrams that were acknowledged
         dwnb | number | Number of downlink datagrams received (unsigned integer)
         txnb | number | Number of packets emitted (unsigned integer)
=end

        def self.from_json
            self.new
        end
    
        def initialize(**param)
            
            
        end
        
        def to_json
            {

            }.to_json
        end
    
    end
    
end
