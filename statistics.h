struct dataperminute
{
    int bytes;      //Bytes received per minute
    int msgs;       //Number of UPD datagrams received per minute
    int maxmsg;     //Bytes of msg with max size in the minute
};

//Function for reset data info:
void resetData (struct dataperminute *data){
    data->bytes = 0;
    data->msgs = 0;
    data->maxmsg = 0;
}

//Function to print new data line in file:
int appendDataToFile (char* filepath, struct dataperminute *data, int diffminutes){
    
    FILE *f = fopen(filepath, "a");
    if (f == NULL){                
        return 0;
    }
    else{
        
        // ------- DATA to FILE -------
        //Timestamp:
        struct tm *tm_struct;
        time_t now = time(0);
        tm_struct = localtime(&now);
        fprintf(f, "%02d:%02d - ", tm_struct->tm_hour, tm_struct->tm_min);
        //Total bytes:
        float totalkb = data->bytes/1024;
        fprintf(f, "Total(KB|B):%.2f|%d,", totalkb, data->bytes);
        //Total messages received:
        fprintf(f, "TotalMsgs:%d,", data->msgs);
        //Max message size:
        float maxmsgsize = data->maxmsg/1024;
        fprintf(f, "MaxMsgSize(KB|B):%.2f|%d,", maxmsgsize,data->maxmsg);
        //Msg mean size:
        float means = data->bytes / data->msgs;
        fprintf(f, "MsgMeanSize(B):%.1f,", means);
        //KB|B per minute:
        float totalkbmin = totalkb / diffminutes;
        float totalbmin = data->bytes / diffminutes;
        fprintf(f, "(KB/min|B/min):%.2f|%.2f,", totalkbmin, totalbmin);
        //KB|B per second:
        float totalkbsec = totalkbmin / 60;
        float totalbsec = totalbmin / 60;
        fprintf(f, "(KB/sec|B/sec):%.2f|%.2f,", totalkbsec, totalbsec);
        //Messages per minute:
        float msgmin = data->msgs / diffminutes;
        fprintf(f, "(Msgs/min):%.2f,", msgmin);
        //Messages per second:
        float msgsec = msgmin / 60;
        fprintf(f, "(Msgs/sec):%.2f", msgsec);
        
        fprintf(f, "\n");
        
    }
    
    fclose(f);
    return 1;
}