//
//  MachProtocol.m
//  obs-mac-virtualcam
//
//  Created by John Boiles  on 5/5/20.
//

#define MACH_SERVICE_NAME @"com.johnboiles.obs-mac-virtualcam.server"

typedef enum {
    //! Initial connect message sent from the client to the server to initate a connection
    MachMsgIdConnect = 1,
    //! Message containing data for a frame
    MachMsgIdFrame = 2,
} MachMsgId;
