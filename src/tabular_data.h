#ifndef TABULAR_DATA_2KJF29SSLFKJ_H
#define TABULAR_DATA_2KJF29SSLFKJ_H

#include <vector>
#include <string>
#include <map>
#include <variant>
#include <set>

#include "CSV.h"

#include "utile.h"


class BinarizedTabularData {
    std::vector<std::vector<std::vector<bool>>> _data; // _data[classe][id][feature] = value
    std::vector<std::string> _binFeature2String;
    std::vector<std::string> _classes;

    unsigned int _numFeatures=0;

public:
    BinarizedTabularData() {

    }

    BinarizedTabularData(const std::vector<std::vector<std::vector<bool>>>& data, const std::vector<std::string> &binFeature2String, const std::vector<std::string> &classes)
        : _data(data), _binFeature2String(binFeature2String), _classes(classes) {

        for(auto &e: data) {
            if(e.size()) {
                _numFeatures = e[0].size();
                break;
            }
        }
    }

    void addIDAsFeatures() {
        unsigned int sizeData = 0;
        for(auto &e: _data) {
            sizeData += e.size();
        }

        unsigned int i=0;
        for(auto &C: _data) {
            for(auto &e: C) {
                assert(e.size() == _numFeatures);
                e.resize(_numFeatures + sizeData, false);
                e[_numFeatures + i] = true;
                i++;
                _binFeature2String.push_back(toString("ID==", i));
            }
        }

        _numFeatures += sizeData;
    }

    void add(const BinarizedTabularData& data2) {
        assert(data2._binFeature2String.size() == _binFeature2String.size());
        assert(data2._classes.size() == _classes.size());
        assert(data2._numFeatures == _numFeatures);

        for(unsigned int c=0; c<_data.size(); c++) {
            _data[c].insert(_data[c].end(), data2._data[c].begin(), data2._data[c].end());
        }
    }

    const auto& getData() const {
        return _data;
    }

    const unsigned int numClasses() const {
        return _data.size();
    }

    const unsigned int numFeatures() const {
        return _numFeatures;
    }

    const std::string & feature2string(unsigned int idFeature) const {
        assert(idFeature < _binFeature2String.size());
        return _binFeature2String[idFeature];
    }

    const std::string& getClassName(unsigned int id) const {
        assert(id < _classes.size());
        return _classes[id];
    }

    std::vector<BinarizedTabularData> partition(unsigned int k, std::optional<unsigned int> seed={}) {
        std::vector<BinarizedTabularData> result;
        if(seed) {
            std::srand(seed.value());
        }

        std::vector< std::tuple< unsigned int,  std::vector<bool> > > all;
        for(unsigned int c=0; c<_data.size(); c++) {
            for(auto e: _data[c]) {
                all.push_back( std::make_tuple(c, e) );
            }
        }
        std::random_shuffle(all.begin(), all.end());

        result.resize(k);
        for(auto &e: result) {
            e._binFeature2String = _binFeature2String;
            e._classes = _binFeature2String;
            e._numFeatures = _numFeatures;
            e._data.resize( numClasses() );
        }

        for(unsigned int i=0; i<all.size(); i++) {
            result[i%k]._data[ std::get<0>(all[i]) ].push_back( std::get<1>(all[i]) );
        }

        return result;
    }
};

class TabularData {

    std::vector< std::vector<std::variant<double, std::string>> > _trainingData;
    std::vector< std::vector<std::variant<double, std::string>> > _testData;

    std::vector< std::set<std::variant<double, std::string>> > _alphabets;
    std::vector<bool> _isDouble;
    std::vector<std::string> _label;

    // Update alphabets and return data
    std::vector< std::vector<std::variant<double, std::string>> > parse(const std::string &file, char sep) {
        CSV csv(file, sep);

        std::vector< std::vector<std::variant<double, std::string>> > data;

        ////////// LABEL ////////////
        {
            csv.next();
            if(_label.size() == 0) {
                _label = csv.get();
            } else {
                assert(_label.size() == csv.get().size());
                for(int i=0; i<_label.size(); i++) {
                    assert( _label[i].compare(csv.get()[i]) == 0 );
                }
            }
            if(csv.size() == 0) {
                std::cerr << "CSV format error: EMPTY!" << std::endl;
                exit(-1);
            }
            assert(_label.size() > 1);
        }

        ///////// FIRST LINE /////////////
        if(_alphabets.size() == 0)
        {
            while(csv.next()) {
                auto line = csv.get();
                if(line.size() == 0)
                    continue;   // Empty line

                if(line.size() != _label.size()) {
                    std::cerr << "CSV format error: " << _label.size() << " columns inspected." << std::endl;
                    exit(-1);
                }

                data.push_back({});
                for(auto &e: line) {
                    auto d = toDouble(e);
                    if(d) {
                        _isDouble.push_back(true);
                        _alphabets.push_back({d.value()});
                        data.back().push_back(d.value());
                    } else {
                        _isDouble.push_back(false);
                        _alphabets.push_back({e});
                        data.back().push_back(e);
                    }
                }
                break;
            }
            if(_alphabets.size() != _label.size()) {
                std::cerr << "CSV format error: NO DATA!" << std::endl;
                exit(-1);
            }
        }

        ///////// OTHER LINE ///////////
        {
            while(csv.next()) {
                auto line = csv.get();

                if(line.size() == 0)
                    continue;   // Empty line

                if(line.size() != _label.size()) {
                    std::cerr << "CSV format error: " << _label.size() << " columns inspected." << std::endl;
                    exit(-1);
                }

                data.push_back({});
                for(unsigned int i=0; i<line.size(); i++) {
                    auto d = toDouble(line[i]);
                    if(d) {
                        assert(_isDouble[i] == true);
                        _alphabets[i].insert(d.value());
                        data.back().push_back(d.value());
                    } else {
                        assert(_isDouble[i] == false);
                        _alphabets[i].insert(line[i]);
                        data.back().push_back(line[i]);
                    }
                }
            }
        }


        assert(_alphabets.size() == _label.size());
        assert(_isDouble.size() == _label.size());

        return data;
    }



public:
    TabularData(const std::string &trainingData, std::optional<std::string> testingData={}, std::optional<unsigned int> idClass={}, char sep=',') {
        _trainingData = parse(trainingData, sep);
        if(testingData) {
            _testData = parse( testingData.value(), sep );
        }
    }

    BinarizedTabularData getBinarizedTraining(bool useEquality, std::optional<unsigned int> idClass={}) {
        return binarized(_trainingData, useEquality, idClass);
    }

    BinarizedTabularData getBinarizedTesting(bool useEquality, std::optional<unsigned int> idClass={}) {
        return binarized(_testData, useEquality, idClass);
    }



private:

    BinarizedTabularData binarized(const std::vector< std::vector<std::variant<double, std::string>> > &dataToBinarized, bool useEquality, std::optional<unsigned int> idClass={}) {
        unsigned int idC = idClass.value_or(_label.size()-1);

        std::vector< std::map< std::variant<double, std::string>, std::vector<bool> > > value2bin; // value2bin[idFeature][valueFeature] = binary value
        std::vector<std::string> binFeature2String;


        value2bin.resize(_alphabets.size());
        for(unsigned int i=0; i<_alphabets.size(); i++) {
            assert(_alphabets[i].size());
            if(i == idClass)
                continue;
            if(_alphabets[i].size() == 1)
                continue;

            if(!_isDouble[i] || useEquality) {

                unsigned int num=0;
                for(auto e: _alphabets[i]) {
                    std::vector<bool> tmp;
                    if(_alphabets[i].size() == 2) {
                        tmp.resize(1, num==0);
                    } else {
                        tmp.resize(_alphabets[i].size(), false);
                        tmp[num] = true;
                    }
                    value2bin[i][e] = tmp;
                    if((_alphabets[i].size() == 2) && (num == 1)) {
                        break;
                    }

                    num++;
                    if(_isDouble[i]) {
                        binFeature2String.push_back(toString(_label[i], "==", std::get<double>(e)));
                    } else {
                        binFeature2String.push_back(toString(_label[i], "==", std::get<std::string>(e)));
                    }
                }

            } else {
                std::vector<bool> tmp;
                tmp.resize(_alphabets[i].size()-1, false);
                unsigned int num=0;
                bool first=true;
                for(auto e: _alphabets[i]) {
                    if(!first) {
                        tmp[num] = true;
                        num++;
                    }
                    first=false;
                    value2bin[i][e] = tmp;

                    if(num == tmp.size()) {
                        break;
                    }
                    binFeature2String.push_back(toString(_label[i], ">", std::get<double>(e)));
                }
            }
        }

        std::map<std::variant<double, std::string>, unsigned int> class2ID;
        unsigned int num=0;
        for(auto &e: _alphabets[idC]) {
            class2ID[e] = num;
            num++;
        }

        std::vector<std::vector<std::vector<bool>>> data; // _data[classe][id][feature] = value
        data.resize(class2ID.size());

        for(auto line: dataToBinarized) {
            std::vector<bool> features;
            for(int i=0; i<_label.size(); i++) {
                if(i == idC)
                    continue;
                std::vector<bool> vBin = value2bin[i][line[i]];

                features.insert(features.end(), vBin.begin(), vBin.end());
            }
            data[ class2ID[line[idC]] ].push_back( features );
        }

        std::vector<std::string> classes;
        if(_isDouble[idC]) {
            for(auto &d: _alphabets[idC]) {
                classes.push_back(toString(std::get<double>(d)));
            }
        } else {
            for(auto &s: _alphabets[idC]) {
                classes.push_back(std::get<std::string>(s));
            }
        }

        return BinarizedTabularData(data, binFeature2String, classes);
    }


};





#endif // TABULAR_DATA_2KJF29SSLFKJ_H
