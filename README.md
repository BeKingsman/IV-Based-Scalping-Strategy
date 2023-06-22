# IV-Based-Scalping-Strategy

The Volatility Mean Reversion scalping strategy aims to capitalise
on the principle that price tends to revert to its mean or average
value after experiencing extreme movements caused by changes in
implied volatility. This strategy assumes that when prices deviate
significantly from the mean, there is a higher probability of a
reversal.
 
 
 ## Reasoning
- When implied volatility increases, it often leads to sharp price movements, pushing prices away from their mean.
- The strategy assumes that after such extreme movements, prices are likely to revert back(Show Correction).
- By identifying these extreme price movements, we can attempt to capture short-term profits as prices reverse.

## Entry Condition
- We will first identify sudden changes in implied volatility.
- If increase in implied volatility in last X days exceeds a threshold, it suggests option is overpriced, so we enter a short position.
- If decrease in implied volatility in last X days exceeds a threshold, it suggests option is underpriced, so we enter a long position.

## Exit Condition
- We exit a long/short position if out target profit is achieved or out stop loss gets hit.

## Report
- The strategy was backtested on 15 year Nifty Options Data.
- Implied volatility was calculated using Black Scholes Model.
- LOOKBACK_PERIOD is kept as 3 days, it is the window to check for sudden changes in implied volatility.
- ENTER_TRIGGER_PERCENTAGE is kept as 20%, it is the change in implied volatility observed to enter a long/short position.
- TARGET_PERCENTAGE is kept as 20%, it is the target profit percentage for our trades.
- STOP_LOSS_PERCENTAGE is kept as 10%, it is the targeted stop loss for our trades.
- Total Number of Trades Executed - 8410
- Number of Profitable Trades - 4034
- Average Profit Per Trade - 57.42%

## Screenshot of sample executed trades
<img width="597" alt="image" src="https://github.com/BeKingsman/IV-Based-Scalping-Strategy/assets/53928332/ef68e453-2f65-423f-8dbe-fdfbf742ee8d">

