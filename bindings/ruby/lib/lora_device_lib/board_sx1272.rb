module LoraDeviceLib

    class Board_sx1272 < Board
    
        REGISTERS = {
        
            :REG_LR_FIFO                                 => 0x00,
            # Common settings
            :REG_LR_OPMODE                               => 0x01,
            :REG_LR_FRFMSB                               => 0x06,
            :REG_LR_FRFMID                               => 0x07,
            :REG_LR_FRFLSB                               => 0x08,
            # Tx settings
            :REG_LR_PACONFIG                             => 0x09,
            :REG_LR_PARAMP                               => 0x0A,
            :REG_LR_OCP                                  => 0x0B,
            # Rx settings
            :REG_LR_LNA                                  => 0x0C,
            # LoRa registers
            :REG_LR_FIFOADDRPTR                          => 0x0D,
            :REG_LR_FIFOTXBASEADDR                       => 0x0E,
            :REG_LR_FIFORXBASEADDR                       => 0x0F,
            :REG_LR_FIFORXCURRENTADDR                    => 0x10,
            :REG_LR_IRQFLAGSMASK                         => 0x11,
            :REG_LR_IRQFLAGS                             => 0x12,
            :REG_LR_RXNBBYTES                            => 0x13,
            :REG_LR_RXHEADERCNTVALUEMSB                  => 0x14,
            :REG_LR_RXHEADERCNTVALUELSB                  => 0x15,
            :REG_LR_RXPACKETCNTVALUEMSB                  => 0x16,
            :REG_LR_RXPACKETCNTVALUELSB                  => 0x17,
            :REG_LR_MODEMSTAT                            => 0x18,
            :REG_LR_PKTSNRVALUE                          => 0x19,
            :REG_LR_PKTRSSIVALUE                         => 0x1A,
            :REG_LR_RSSIVALUE                            => 0x1B,
            :REG_LR_HOPCHANNEL                           => 0x1C,
            :REG_LR_MODEMCONFIG1                         => 0x1D,
            :REG_LR_MODEMCONFIG2                         => 0x1E,
            :REG_LR_SYMBTIMEOUTLSB                       => 0x1F,
            :REG_LR_PREAMBLEMSB                          => 0x20,
            :REG_LR_PREAMBLELSB                          => 0x21,
            :REG_LR_PAYLOADLENGTH                        => 0x22,
            :REG_LR_PAYLOADMAXLENGTH                     => 0x23,
            :REG_LR_HOPPERIOD                            => 0x24,
            :REG_LR_FIFORXBYTEADDR                       => 0x25,
            :REG_LR_FEIMSB                               => 0x28,
            :REG_LR_FEIMID                               => 0x29,
            :REG_LR_FEILSB                               => 0x2A,
            :REG_LR_RSSIWIDEBAND                         => 0x2C,
            :REG_LR_DETECTOPTIMIZE                       => 0x31,
            :REG_LR_INVERTIQ                             => 0x33,
            :REG_LR_DETECTIONTHRESHOLD                   => 0x37,
            :REG_LR_SYNCWORD                             => 0x39,
            :REG_LR_INVERTIQ2                            => 0x3B,

            # end of documented register in datasheet
            # I/O settings
            :REG_LR_DIOMAPPING1                          => 0x40,
            :REG_LR_DIOMAPPING2                          => 0x41,
            # Version
            :REG_LR_VERSION                              => 0x42,
            # Additional settings
            :REG_LR_AGCREF                               => 0x43,
            :REG_LR_AGCTHRESH1                           => 0x44,
            :REG_LR_AGCTHRESH2                           => 0x45,
            :REG_LR_AGCTHRESH3                           => 0x46,
            :REG_LR_PLLHOP                               => 0x4B,
            :REG_LR_TCXO                                 => 0x58,
            :REG_LR_PADAC                                => 0x5A,
            :REG_LR_PLL                                  => 0x5C,
            :REG_LR_PLLLOWPN                             => 0x5E,
            :REG_LR_FORMERTEMP                           => 0x6C,
        }
    
        def initialize
            @type = :LORA_RADIO_SX1272
        end
    
        def write(data)
        
            if @select
        
                msg = "wrote 0x#{ "%02X" % data }"
            
                case @opcode
                when nil
                    @opcode = data & 0x7f
                    @reg = REGISTERS.key(@opcode)
                    @mode = ( (data & 0x80) == 0 ? :read : :write )                    
                    msg << " ( #{@mode} #{ ( @reg ? @reg : "?" ) } )"                    
                else                
                    @write_buffer << data.chr                                
                end
                
                puts msg
                
            end
            
        end
    
        def select(state)
        
            if @select
            
                if state
                
                    raise "double select"
                
                else
            
                    puts "release"
                    puts "========================"
                    
                    
                    if @reg
                    
                    
                        
                    
                    
                    end
                    
                    
                    
                    @opcode = nil
                    
                end
                
            else
            
                if state
                
                    @write_buffer = ""
                    
                    puts "========================"
                    puts "select"
                    
                else
                
                    raise "double release" 
                    
                end
                    
            end
    
            @select = state
    
        end
        
        
        private
        
            
        
        
    end


end
