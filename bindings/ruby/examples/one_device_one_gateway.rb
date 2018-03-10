require 'ldl'

include LDL

LDL::Logger = CompositeLogger
LDL::Logger << ::Logger.new(STDOUT).tap do |log|
  log.formatter = LDL::LOG_FORMATTER
  log.sev_threshold = Logger::INFO
end

LDL::SystemTime = Clock.new

broker = Broker.new

FrameLogger.new(broker, File.open('frame.log', 'w'))

gateway = Gateway.new(broker, EUI64.new(ENV["GATEWAY_EUI"]), name: 'gateway')

device = Device.new(broker, 
    devEUI: EUI64.new(ENV["DEV_EUI"]),
    appEUI: EUI64.new(ENV["APP_EUI"]), 
    appKey: Key.new(ENV["APP_KEY"]), 
    name: "device"
) do |d|

  sleep 1

  d.mac.on_receive { |port, message| puts "port#{port}: #{message}" }
          
  begin   
      
    puts "joining..."
    
    d.mac.join
    
    puts "join complete"

    counter = 0; port = 1
    
    loop do
    
      SystemTime.wait(d.mac.ticksUntilNextChannel)
      
      puts "sending message '#{counter}' to port #{port}..."
  
      d.mac.data port, counter.to_s
      
      puts "send complete"
      
      counter += 1
        
    end
      
  rescue JoinTimeout            
  
    puts "join failed"
      
  end
    
end

[LDL::SystemTime, gateway, device].each(&:start)
    
begin
  sleep 
rescue Interrupt
end

[LDL::SystemTime, gateway, device].each(&:stop)

exit
