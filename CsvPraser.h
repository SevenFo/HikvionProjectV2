//
// Created by siky on 2022/2/18.
//

#ifndef MFCC_CSVPRASER_H
#define MFCC_CSVPRASER_H

#include <iostream>
#include "string"
#include "fstream"
#include "vector"
#include "array"


typedef std::vector<std::string> row_content;

class CsvPraser {

public:
    CsvPraser(const std::string &file_name, const char split_char){
        _fc.open(file_name);
        if(!_fc.is_open())
        {
            std::cout <<"open "<<file_name<<" filed"<<std::endl;
            exit(-1);
        }
        std::string line;
        std::getline(_fc,line);
        _heads = _split(line,split_char);

        while(std::getline(_fc,line))
        {
            _content.push_back(_split(line,split_char));
        }

    }
    ~CsvPraser()
    {
        _fc.close();
    }

    const std::vector<row_content> get_content(){
        return _content;
    }

private:
    std::vector<std::string> _split(const std::string & data,const char split_char)
    {
        std::size_t last_pose=0,pos = 0;
        row_content items;
        do
        {
            pos = data.find(split_char,last_pose);
            items.push_back(std::string(data.data(),last_pose,pos==std::string::npos? data.size()-last_pose:pos-last_pose));
            last_pose = pos+1;
        }while(pos != std::string::npos);

        return items;
    }


private:
    std::ifstream _fc;
    row_content _heads;
    std::vector<row_content> _content;
};


#endif //MFCC_CSVPRASER_H
