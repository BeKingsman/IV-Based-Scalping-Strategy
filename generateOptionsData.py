import pandas as pd
import numpy as np
from py_vollib.black_scholes_merton.implied_volatility import implied_volatility

def getNiftyData():
    niftyData = pd.read_csv("data/nifty_15 yr_data.csv")
    niftyData = niftyData[["Date","Symbol",  "Expiry", "Option Type",  "Strike Price","Close","Settle Price","Underlying"]]
    niftyData['Date'] = pd.to_datetime(niftyData['Date'])
    niftyData['Expiry'] = pd.to_datetime(niftyData['Expiry'])
    niftyData.set_index("Date",inplace=True)
    niftyData["Option Type"] = niftyData["Option Type"].replace(["PE"],"p")
    niftyData["Option Type"] = niftyData["Option Type"].replace(["CE"],"c")
    niftyData= niftyData.rename(columns={"Underlying" : "Stock Price"})
    return niftyData


def getRiskFreeRateData():
    riskFreeRateData = pd.read_csv("data/India 15-Year Bond Yield Historical Data.csv")
    riskFreeRateData['Date'] = pd.to_datetime(riskFreeRateData['Date'])
    riskFreeRateData = riskFreeRateData[["Date","Price"]]
    riskFreeRateData["Price"] = riskFreeRateData["Price"]/100
    riskFreeRateData.set_index("Date",inplace=True)
    riskFreeRateData = riskFreeRateData.rename(columns={"Price":"Risk Free Rate"})
    return riskFreeRateData


countIVErrors = 0
def findImpliedVolatality(x):
    global countIVErrors
    try:
        return 100*implied_volatility(float(x["Settle Price"]), float(x["Stock Price"]), float(x["Strike Price"]), (x["Expiry"]-x["date"]).days/365, float(x["Risk Free Rate"]), 0, x["Option Type"])
    except:
        countIVErrors+=1
        return np.nan
    
def getData():
    niftyData = getNiftyData()
    riskFreeRateData = getRiskFreeRateData()
    niftyData = niftyData.join(riskFreeRateData)
    niftyData["date"] = niftyData.index
    niftyData = niftyData.dropna()
    niftyData = niftyData.sort_values(by=['Expiry','Option Type','Strike Price'])
    niftyData["Implied Volatality"] = niftyData.apply(findImpliedVolatality, axis=1)
    niftyData = niftyData.dropna()
    print(niftyData.head(10))
    return niftyData



niftyData = getData()
niftyData.to_csv("data/projectDataset.csv",index=False)