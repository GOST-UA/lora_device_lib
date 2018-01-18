module LDL

    # used to indicate the state is not right (e.g. you cannot personalise a joined device)
    class StateError < StandardError
    end    
    
    # timeout condition while waiting for join confirmation
    class JoinTimeout < StandardError
    end
    
    #  timeout condition while waiting for confirmed send confirmation
    class SendTimeout < StandardError
    end
    
    # generic run-time error...
    class Error < StandardError
    end

end
