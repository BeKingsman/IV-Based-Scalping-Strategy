#include<iostream>
#include<vector>
#include<queue>
#include<map>
#include<fstream>
#include<string>
#include <sstream>
#include<memory>

#define DEBUG true
#define PRINT_TRADES true

template<typename T>
void printMessage(T msg, bool nextLine = true){
    std::cout<<msg;
    if(nextLine)std::cout<<std::endl;
    else std::cout<<" ";
} 

struct OptionDataPoint{
    std::string expiry;
    std::string date;
    std::string optionType;
    float strikePrice;
    float optionClosePrice;
    float optionSettlePrice;
    float stockPrice;
    float riskFreeRate;
    float impliedVolatality;


    void setData(const std::string &line){
        std::string word;
        std::stringstream s(line);
        int column=0;
        while (getline(s, word,',')) {
            if(column==1)expiry = word;
            if(column==2)optionType = word;
            if(column==8)date = word;

            if(column==3)strikePrice = stof(word);
            if(column==4)optionClosePrice = stof(word);
            if(column==5)optionSettlePrice = stof(word);
            if(column==6)stockPrice = stof(word);
            if(column==7)riskFreeRate = stof(word);
            if(column==9)impliedVolatality = stof(word);
            column++;
        }
    }

    inline void printData() const {
        std::cout<<"Date : "<<this->date<<", Expiry : "<<this->expiry<<", Option Type : "<<this->optionType<<", Strike Price : "<<this->strikePrice<<", Close Price : "<<this->optionClosePrice<<", Stock price : "<<this->stockPrice<<", Risk free rate : "<<this->riskFreeRate<<", Implied Volatility : "<<this->impliedVolatality<<std::endl;
    }

};


class Data{
    public:
        std::string symbolName;
        std::vector<OptionDataPoint> symbolData;
        int numberOfRows;

        Data(std::string symbolName, std::string fileName)
            : symbolName(symbolName), numberOfRows(0){
            parseData(fileName);
        }

        void parseData(const std::string &fileName){
            numberOfRows = 0;
            symbolData.clear();

            std::fstream fin;
            fin.open(fileName, std::ios::in);
            std::string temp,line;

            while (fin >> temp) {
                OptionDataPoint row;
                row.setData(temp);
                if(row.expiry.size()){
                    symbolData.push_back(row);
                    numberOfRows++;          
                }
                else{
                    std::cout<<"LINE ERROR"<<std::endl;
                }
            }
        }

        void printData() const {
            for(int row=0;row<numberOfRows;row++){
                symbolData[row].printData();
            }
        }

        static int daysBetweenDates(std::string date1, std::string date2){
            std::stringstream ss(date1 + "-" + date2);
            int year, month, day;
            char hyphen;
        
            ss >> year >> hyphen >> month >> hyphen >> day;
            struct tm starttm = { 0, 0, 0, day, month - 1, year - 1900 };
            time_t start = mktime(&starttm);
        
            ss >> hyphen >> year >> hyphen >> month >> hyphen >> day;
            struct tm endtm = { 0, 0, 0, day, month - 1, year - 1900 };
            time_t end = mktime(&endtm);

            return abs(end - start) / 86400;
        }

        int* getIvKMax(int K) const {
            std::deque<int> Qi(K);
            int i,N= numberOfRows;
            float mx=INT_MIN;
            int *kmax = new int[N];

            for (i=0; i < N; ++i) {
                while ((!Qi.empty()) && (daysBetweenDates(symbolData[Qi.front()].date, symbolData[i].date)>K || symbolData[Qi.front()].strikePrice!=symbolData[i].strikePrice || symbolData[Qi.front()].expiry!=symbolData[i].expiry || symbolData[Qi.front()].optionType!=symbolData[i].optionType))
                    Qi.pop_front();
                while ((!Qi.empty()) && symbolData[i].impliedVolatality >= symbolData[Qi.back()].impliedVolatality)
                    Qi.pop_back();
                Qi.push_back(i);
                kmax[i]=Qi.front();
            }

            return kmax;
        }

        int* getIvKMin(int K) const {
            std::deque<int> Qi(K);
            int i,N=numberOfRows;
            float mn=INT_MAX;
            int *kmin = new int[N];

            for (i=0; i < N; ++i) {
                while ((!Qi.empty()) && (daysBetweenDates(symbolData[Qi.front()].date, symbolData[i].date)>K || symbolData[Qi.front()].strikePrice!=symbolData[i].strikePrice || symbolData[Qi.front()].expiry!=symbolData[i].expiry || symbolData[Qi.front()].optionType!=symbolData[i].optionType))
                    Qi.pop_front();
                while ((!Qi.empty()) && symbolData[i].impliedVolatality <= symbolData[Qi.back()].impliedVolatality)
                    Qi.pop_back();
                Qi.push_back(i);
                kmin[i]=Qi.front();
            }

            return kmin;
        }

        ~Data(){
            if(DEBUG)printMessage("Deconstructing Data Object");
        }
};


class Strategy{
    public:
        std::string strategyName;
        std::map<std::string,int> strategyParams;

        virtual int8_t* getTradeSignals(Data &data) = 0;

        static float findPercentageChange(const float &startValue, const float &endValue){
            return ((endValue-startValue)*100)/startValue;
        }

        virtual ~Strategy(){
            
        }
};


class IVScalpingStrategy : public Strategy{

    public:
        IVScalpingStrategy(){
            this->strategyName="IV Scalping Strategy";
            strategyParams["LOOKBACK_PERIOD"]=3;
            strategyParams["ENTER_TRIGGER_PERCENTAGE"]=20;
            strategyParams["TARGET_PERCENTAGE"]=20;
            strategyParams["STOP_LOSS_PERCENTAGE"]=10;
        }

        IVScalpingStrategy(std::string strategyName, int lookBackPeriod, int entryTrigger,int targetPercentage, int stopLoss){
            this->strategyName=strategyName;
            strategyParams["LOOKBACK_PERIOD"]=lookBackPeriod;
            strategyParams["ENTER_TRIGGER_PERCENTAGE"]=entryTrigger;
            strategyParams["TARGET_PERCENTAGE"]=targetPercentage;
            strategyParams["STOP_LOSS_PERCENTAGE"]=stopLoss;
        }

        static bool comparator(OptionDataPoint &optionDataPoint1, OptionDataPoint &optionDataPoint2){
            if(optionDataPoint1.expiry != optionDataPoint2.expiry) return optionDataPoint1.expiry<optionDataPoint2.expiry;
            if(optionDataPoint1.strikePrice != optionDataPoint2.strikePrice) return optionDataPoint1.strikePrice<optionDataPoint2.strikePrice;
            if(optionDataPoint1.optionType != optionDataPoint2.optionType)return optionDataPoint1.optionType<optionDataPoint2.optionType;
            if(optionDataPoint1.date != optionDataPoint2.date) return optionDataPoint1.date<optionDataPoint2.date;
            return false;
        }

        int8_t* getTradeSignals(Data &data){
            sort(data.symbolData.begin(), data.symbolData.end(), IVScalpingStrategy::comparator);

            enum TradeState{ NO_POSITION=0, LONG_POSITION=1, SHORT_POSITION=2 };

            int8_t* signals = new int8_t[data.numberOfRows];
            TradeState state = NO_POSITION;
            int tradePrice = 0;
            int tradeEntryIndex = -1;

            int* kmax = data.getIvKMax(strategyParams["LOOKBACK_PERIOD"]);
            int* kmin = data.getIvKMin(strategyParams["LOOKBACK_PERIOD"]);

            for(int i=0;i<data.numberOfRows;i++){
                int maxPercentIncrease = findPercentageChange(data.symbolData[kmin[i]].impliedVolatality,data.symbolData[i].impliedVolatality);
                int maxPercentDecrease = -1 * findPercentageChange(data.symbolData[kmax[i]].impliedVolatality,data.symbolData[i].impliedVolatality);
                
                if(state==NO_POSITION){
                    if(maxPercentIncrease >= strategyParams["ENTER_TRIGGER_PERCENTAGE"]){
                        state=SHORT_POSITION;
                        signals[i]=2;
                        tradePrice = data.symbolData[i].optionClosePrice;
                        tradeEntryIndex = i;
                    }
                    else if(maxPercentDecrease >= strategyParams["ENTER_TRIGGER_PERCENTAGE"]){
                        state=LONG_POSITION;
                        signals[i]=1;
                        tradePrice = data.symbolData[i].optionClosePrice;
                        tradeEntryIndex = i;

                    }
                    else{
                        signals[i]=0;
                    }
                }
                else if(state==LONG_POSITION){
                    if(data.symbolData[tradeEntryIndex].strikePrice!=data.symbolData[i].strikePrice || data.symbolData[tradeEntryIndex].expiry!=data.symbolData[i].expiry || data.symbolData[tradeEntryIndex].optionType!=data.symbolData[i].optionType){
                        state = NO_POSITION;
                        signals[tradeEntryIndex]=0;
                        signals[i]=0;
                        continue;
                    }
                    if(findPercentageChange(tradePrice,data.symbolData[i].optionClosePrice)>=strategyParams["TARGET_PERCENTAGE"] ||  
                        (-1*findPercentageChange(tradePrice,data.symbolData[i].optionClosePrice)>=strategyParams["STOP_LOSS_PERCENTAGE"])){
                        state=NO_POSITION;
                        signals[i]=-1;

                    }
                    else signals[i]=0;
                }
                else if(state==SHORT_POSITION){
                    if(data.symbolData[tradeEntryIndex].strikePrice!=data.symbolData[i].strikePrice || data.symbolData[tradeEntryIndex].expiry!=data.symbolData[i].expiry || data.symbolData[tradeEntryIndex].optionType!=data.symbolData[i].optionType){
                        state = NO_POSITION;
                        signals[tradeEntryIndex]=0;
                        signals[i]=0;
                        continue;
                    }
                    if((-1*findPercentageChange(tradePrice,data.symbolData[i].optionClosePrice))>=strategyParams["TARGET_PERCENTAGE"] || 
                        (findPercentageChange(tradePrice,data.symbolData[i].optionClosePrice)>=strategyParams["STOP_LOSS_PERCENTAGE"])){
                        state=NO_POSITION;
                        signals[i]=-2;

                    }
                    else signals[i]=0;
                }
            }

            delete[] kmax;
            delete[] kmin;

            return signals;
        }

        ~IVScalpingStrategy(){
            strategyParams.clear();
            if(DEBUG)printMessage("Deconstructing Strategy Object");
        }

};



class Backtest{
    std::shared_ptr<Strategy> strategyInstance;
    std::shared_ptr<Data> dataInstance;
    int8_t* tradeSignals;

    public:
        Backtest(std::shared_ptr<Strategy> &strategyInstance, std::shared_ptr<Data> &dataInstance)
            :strategyInstance(strategyInstance), dataInstance(dataInstance), tradeSignals(nullptr){
            
        }

        void printResults(const int &totalTrades, const int &numOfProfitableTrades, const float &totalProfitPercent){
            std::cout<<"\n*************************************************************\n";
            std::cout<<"Symbol: "<<dataInstance->symbolName<<std::endl;
            std::cout<<"Strategy: "<<strategyInstance->strategyName<<std::endl;
            std::cout<<"Total Trades Taken: "<<totalTrades<<std::endl;
            std::cout<<"Number Of Profitable Trades: "<<numOfProfitableTrades<<std::endl;
            std::cout<<"Average Profit Percentage Per Trade: "<<float(totalProfitPercent)/float(totalTrades)<<std::endl;
            std::cout<<"*************************************************************\n";
        }

        void evaluateResults(){
            float openPrice;
            float totalProfitPercent=0;
            int numOfProfitableTrades=0,totalTrades=0;
            for(int i=0;i<dataInstance->numberOfRows;i++){
                if(tradeSignals[i]==1){
                    openPrice = dataInstance->symbolData[i].optionClosePrice;
                    if(PRINT_TRADES){
                        std::cout<<"\n*************************************************************\n";
                        std::cout<<"Entering Long Position\n";
                        dataInstance->symbolData[i].printData();
                        std::cout<<"*************************************************************\n";
                    }
                }
                else if(tradeSignals[i]==-1){
                    float profit = Strategy::findPercentageChange(openPrice,dataInstance->symbolData[i].optionClosePrice);
                    totalProfitPercent+=profit;
                    totalTrades+=1;
                    if(profit>0)numOfProfitableTrades+=1;
                    if(PRINT_TRADES){
                        std::cout<<"\n*************************************************************\n";
                        std::cout<<"Exitting Long Position\n";
                        dataInstance->symbolData[i].printData();
                        std::cout<<"Profit : "<<profit<<"%\n";
                        std::cout<<"*************************************************************\n";
                    }
                }
                else if(tradeSignals[i]==2){
                    openPrice = dataInstance->symbolData[i].optionClosePrice;
                    if(PRINT_TRADES){
                        std::cout<<"\n*************************************************************\n";
                        std::cout<<"Entering Short Position\n";
                        dataInstance->symbolData[i].printData();
                        std::cout<<"*************************************************************\n";
                    }
                }
                else if(tradeSignals[i]==-2){
                    float profit = Strategy::findPercentageChange(dataInstance->symbolData[i].optionClosePrice, openPrice);
                    totalProfitPercent+=profit;
                    totalTrades+=1;
                    if(profit>0)numOfProfitableTrades+=1;
                    if(PRINT_TRADES){
                        std::cout<<"\n*************************************************************\n";
                        std::cout<<"Exitting Short Position\n";
                        dataInstance->symbolData[i].printData();
                        std::cout<<"Profit : "<<profit<<"%\n";
                        std::cout<<"*************************************************************\n";
                    }
                }
            }
            printResults(totalTrades,numOfProfitableTrades,totalProfitPercent);
        }

        void runBacktest(){
            tradeSignals = strategyInstance->getTradeSignals(*dataInstance);
            evaluateResults();
        }

        ~Backtest(){
            delete tradeSignals;
            if(DEBUG)printMessage("Deconstructing Backtest Object");
        }
};



int main()
{

    std::shared_ptr<Strategy> ivScalpingStrategyInstance = std::make_shared<IVScalpingStrategy>();
    std::shared_ptr<Data> dataInstance = std::make_shared<Data>("NIFTY","data/projectDataset.csv");

    std::unique_ptr<Backtest> backtestInstance = std::unique_ptr<Backtest>(new Backtest(ivScalpingStrategyInstance,dataInstance));
    backtestInstance->runBacktest();

    return 0;
}

