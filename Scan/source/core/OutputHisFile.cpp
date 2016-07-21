#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>

#include "OutputHisFile.hpp"

drr_entry *OutputHisFile::find_drr_in_list(unsigned int hisID_){
    // Search for the specified histogram in the .drr entry list
    for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
        if((*iter)->hisID == hisID_){
            return (*iter);
        }
    }
    
    // Check if this his ID is in the bad histogram list
    bool in_bad_list = false;
    for(std::vector<unsigned int>::iterator iter = failed_fills.begin(); iter != failed_fills.end(); iter++){
        if(*iter == hisID_){
            in_bad_list = true;
            break;
        }
    }
    if(!in_bad_list){ failed_fills.push_back(hisID_); }
    
    return NULL;
}

void OutputHisFile::Flush(){
    if(debug_mode){ std::cout << "debug: Flushing histogram entries to file.\n"; }
    
    if(writable){ // Do the filling
        for(std::vector<fill_queue*>::iterator iter = fills_waiting.begin(); iter != fills_waiting.end(); iter++){
            if(!(*iter)->good){ continue; }
            
            current_entry = (*iter)->entry;
            current_entry->good_counts++;
            
            // Seek to the specified bin
            ofile.seekg(current_entry->offset*2 + (*iter)->byte, std::ios::beg); // input offset
            
            unsigned short sval = 0;
            unsigned int ival = 0;
            
            // Overwrite the bin value
            if(current_entry->use_int){
                // Get the original value of the bin
                ofile.read((char*)&ival, 4);
                ival += (*iter)->weight;
		
                // Set the new value of the bin
                ofile.seekp(current_entry->offset*2 + (*iter)->byte, std::ios::beg); // output offset
                ofile.write((char*)&ival, 4);
            }
            else{
                // Get the original value of the bin
                ofile.read((char*)&sval, 2);
                sval += (short)(*iter)->weight;
		
                // Set the new value of the bin
                ofile.seekp(current_entry->offset*2 + (*iter)->byte, std::ios::beg); // output offset
                ofile.write((char*)&sval, 2);
            }
        }
    }
    else if(debug_mode){ std::cout << "debug: Output file is not writable!\n"; }
    
    // Delete the drr_entries in the fill_queue vector
    for(std::vector<fill_queue*>::iterator iter = fills_waiting.begin(); iter != fills_waiting.end(); iter++){
        delete (*iter);
    }
    fills_waiting.clear();
    
    Flush_count = 0;
}

OutputHisFile::OutputHisFile(){
    fname = "";
    writable = false;
    finalized = false;
    existing_file = false;
    Flush_wait = 100000;
    Flush_count = 0;
    total_his_size = 0;
    
    initialize();
}

OutputHisFile::OutputHisFile(std::string fname_prefix){
    fname = "";
    writable = false;
    finalized = false;
    existing_file = false;
    Flush_wait = 100000;
    Flush_count = 0;
    total_his_size = 0;
    
    initialize();
    Open(fname_prefix);
}

OutputHisFile::~OutputHisFile(){
    Close();
}

size_t OutputHisFile::push_back(drr_entry *entry_){
    if(!entry_){ 
        if(debug_mode){ std::cout << "debug: OutputHisFile::push_back was passed a NULL pointer!\n"; }
        return 0; 
    }
    else if(!writable || finalized){ 
        if(debug_mode){ std::cout << "debug: The .drr and .his files have already been finalized and are locked!\n"; }
        return 0; 
    }
    
    // Search for existing histogram with the same id
    if(find_drr_in_list(entry_->hisID)){
        if(debug_mode){ std::cout << "debug: His id = " << entry_->hisID << " is already in the drr entry list!\n"; }
	
        return false;
    }
    
    // Seek to the end of this histogram file
    ofile.seekp(0, std::ios::end);
    entry_->offset = (size_t)ofile.tellp()/2; // Set the file offset (in 2 byte words)
    drr_entries.push_back(entry_);
    
    if(debug_mode){	std::cout << "debug: Extending .his file by " << entry_->total_size << " bytes for his ID = " << entry_->hisID << " i.e. '" << rstrip(entry_->title) << "'\n"; }
    
    char *block = new char[entry_->total_size];	
    memset(block, 0x0, entry_->total_size);
    ofile.write(block, entry_->total_size);
    delete[] block;
    
    ofile.seekp(0, std::ios::end);
    total_his_size = ofile.tellp();
    
    return entry_->total_size;
}

bool OutputHisFile::Finalize(bool make_list_file_/*=false*/, const std::string &descrip_/*="RootPixieScan .drr file"*/){
    if(!writable || finalized){ 
        if(debug_mode){ std::cout << "debug: The .drr and .his files have already been finalized and are locked!\n"; }
        return false; 
    }
    
    bool retval = true;
    
    set_char_array(initial, "HHIRFDIR0001", 12);
    set_char_array(description, descrip_, 40);
    
    if(debug_mode){ std::cout << "debug: NHIS = " << drr_entries.size() << std::endl; }
    nHis = drr_entries.size(); 
    nHWords = (128 * (1 + drr_entries.size()) + drr_entries.size() * 4)/2;
    
    time_t rawtime;
    struct tm * timeinfo;
    
    time(&rawtime);
    timeinfo = localtime (&rawtime);
    
    date[0] = 0;
    date[1] = timeinfo->tm_year + 1900; // tm_year measures the year from 1900
    date[2] = timeinfo->tm_mon; // tm_mon ranges from 0 to 11
    date[3] = timeinfo->tm_mday;
    date[4] = timeinfo->tm_hour;
    date[5] = timeinfo->tm_min;
    
    // Write the .drr file
    std::ofstream drr_file((fname+".drr").c_str(), std::ios::binary);
    if(drr_file.good()){
        char dummy = 0x0;
        int his_id;
	
        // Write the 128 byte drr header
        drr_file.write(initial, 12);
        drr_file.write((char*)&nHis, 4);
        drr_file.write((char*)&nHWords, 4);
        for(int i = 0; i < 6; i++){ drr_file.write((char*)&date[i], 4); }
        for(int i = 0; i < 44; i++){ drr_file.write(&dummy, 1); } // add the trailing garbage
        drr_file.write(description, 40);
        
        // Write the drr entries
        for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
            if(debug_mode){ std::cout << "debug: Writing .drr entry for his id = " << (*iter)->hisID << std::endl; }
            (*iter)->print_drr(&drr_file);
        }
	
        // Write the histogram IDs
        for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
            his_id = (*iter)->hisID;
            drr_file.write((char*)&his_id, 4);
        }
    }
    else{
        if(debug_mode){ std::cout << "debug: Failed to open the .drr file for writing!\n"; }
        retval = false;
    }
    drr_file.close();
    
    // Write the .list file (I'm trying to preserve the format of the original file)
    std::ofstream list_file((fname+".list").c_str());
    if(list_file.good()){
        int temp_count = 0;
        list_file << std::setw(7) << drr_entries.size() << " HISTOGRAMS," << std::setw(13) << total_his_size/2 << " HALF-WORDS\n ID-LIST:\n";
        for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
            if(temp_count % 8 == 0 && temp_count != 0){ list_file << std::endl; }
            list_file << std::setw(8) << (*iter)->hisID;
            temp_count++;
        }
        list_file << "\n  HID  DIM HWPC  LEN(CH)   COMPR  MIN   MAX   OFFSET    TITLE\n";
        for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
            (*iter)->print_list(&list_file);
        }
    }
    else{
        if(debug_mode){ std::cout << "debug: Failed to open the .list file for writing!\n"; }
        retval = false;
    }
    list_file.close();	
    
    finalized = true;
    
    return retval;
}

bool OutputHisFile::Fill(unsigned int hisID_, unsigned int x_, unsigned int y_, unsigned int weight_/*=1*/){
    if(!writable){ return false; }
    
    drr_entry *temp_drr = find_drr_in_list(hisID_);
    if(temp_drr){
        unsigned int bin;
        temp_drr->total_counts++;
        if(!temp_drr->find_bin((unsigned int)(x_/temp_drr->comp[0]), (unsigned int)(y_/temp_drr->comp[1]), bin)){ return false; }		
        
        // Push this fill into the queue
        fill_queue *fill = new fill_queue(temp_drr, bin, weight_);
        fills_waiting.push_back(fill);
        
        if(++Flush_count >= Flush_wait){ Flush(); }
    }
    
    return false;
}

bool OutputHisFile::FillBin(unsigned int hisID_, unsigned int x_, unsigned int y_, unsigned int weight_){
    if(!writable){ return false; }
    
    drr_entry *temp_drr = find_drr_in_list(hisID_);
    if(temp_drr){
        unsigned int bin;
        temp_drr->total_counts++;
        if(!temp_drr->get_bin(x_, y_, bin)){ return false; }
	
        // Push this fill into the queue
        fill_queue *fill = new fill_queue(temp_drr, bin, weight_);
        fills_waiting.push_back(fill);
        
        if(++Flush_count >= Flush_wait){ Flush(); }
        return true;
    }
    
    return false;
}

bool OutputHisFile::Zero(unsigned int hisID_){
    if(!writable){ return false; }
    
    drr_entry *temp_drr = find_drr_in_list(hisID_);
    if(temp_drr){
        ofile.seekp(temp_drr->offset*2, std::ios::beg);
	
        char *block = new char[temp_drr->total_size];	
        memset(block, 0x0, temp_drr->total_size);
        ofile.write(block, temp_drr->total_size);
        delete[] block;
	
        return true;
    }
    
    return false;
}

bool OutputHisFile::Zero(){
    if(!writable){ return false; }
    
    for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
        ofile.seekp((*iter)->offset*2, std::ios::beg);
	
        char *block = new char[(*iter)->total_size];	
        memset(block, 0x0, (*iter)->total_size);
        ofile.write(block, (*iter)->total_size);
        delete[] block;
    }
    
    return true;
}

bool OutputHisFile::Open(std::string fname_prefix){
    if(writable){ 
        if(debug_mode){ std::cout << "debug: The .his file is already open!\n"; }
        return false; 
    }
    
    fname = fname_prefix;
    existing_file = false;
    
    //touch.close();
    ofile.open((fname+".his").c_str(), std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
    return (writable = ofile.good());
}

void OutputHisFile::Close(){
    Flush();
    
    if(!finalized){ Finalize(); }
    
    // Write the .log file
    std::ofstream log_file((fname+".log").c_str());
    if(log_file.good()){
        log_file << "  HID      TOTAL      GOOD\n\n";
        for(std::vector<drr_entry*>::iterator iter = drr_entries.begin(); iter != drr_entries.end(); iter++){
            log_file << std::setw(5) << (*iter)->hisID << std::setw(10) << (*iter)->total_counts << std::setw(10) << (*iter)->good_counts << std::endl;
        }
        log_file << "\nFailed histogram fills:\n\n";
        for(std::vector<unsigned int>::iterator iter = failed_fills.begin(); iter != failed_fills.end(); iter++){
            log_file << std::setw(5) << *iter << std::endl;
        }
    }
    else if(debug_mode){ std::cout << "debug: Failed to open the .log file for writing!\n"; }
    log_file.close();
    
    // Clear the .drr entries in the entries vector
    clear_drr_entries();
    
    writable = false;
    ofile.close();
}
