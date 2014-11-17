Params
    Numeric RiskRatio(1);                   
    Numeric ATRLength(20);                  
    Numeric boLength(20);                   
    Numeric fsLength(55);                   
    Numeric teLength(10);                   
    Bool LastProfitableTradeFilter(True);   
    Numeric ReEntryLimit(4);
    String FileFolder("C:\\TBLog");
Vars                 
    String LogPath;
    String ParamPath;
    String TempOrderSide;
    NumericSeries AvgTR;     
    Numeric N;                              
    Numeric TurtleUnits;                    
    NumericSeries DonchianHi;               
    NumericSeries DonchianLo;               
    NumericSeries fsDonchianHi;             
    NumericSeries fsDonchianLo;             
    Numeric ExitHighestPrice;               
    Numeric ExitLowestPrice;                
    Numeric OrderPrice;  
    Numeric OrderQuantity(0);
    Numeric PreEntryPrice(0);
    Numeric AvgCostPrice(0);
    Bool PreBreakoutFailure(False);
    Bool DeleteOrderSuccess(False);
    Bool ClearToGo(True);
    Numeric i;
    Numeric j;
    Numeric EntryNumber(0);
    Numeric TempEntryNumber;
    Numeric MaxSlippage;
    Numeric ContractMargin;
    Numeric OrderSideMultiplier(0);
Begin
    If(BarStatus == 0)
    {
      PreEntryPrice = InvalidNumeric;
      PreBreakoutFailure = false;
    } 
// Calculate N and Max Slippage
    AvgTR = XAverage(TrueRange, ATRLength);
    N = AvgTR[1]; 
    MaxSlippage = 0.05 * N;
 
    DonchianHi = HighestFC(High[1], boLength);
    DonchianLo = LowestFC(Low[1], boLength);
    fsDonchianHi = HighestFC(High[1], fsLength);
    fsDonchianLo = LowestFC(Low[1], fsLength); 
    ExitLowestPrice = LowestFC(Low[1], teLength);
    ExitHighestPrice = HighestFC(High[1], teLength);
 
    LogPath = FileFolder + "\\TBLog_" + Symbol() + "_" +SymbolName() + "_" + Text(CurrentDate()) + ".csv";
    ParamPath = FileFolder + "\\TurtleSetup.csv";
    FileAppend(LogPath, "========") ;
 
 If ( !CanTrade() )
 {
  FileAppend(LogPath, 
   "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
   ", CanNotTrade") ;
 }
 Else If ( A_AccountID() == "" || A_AccountID == InvalidString )
 {
  FileAppend(LogPath,
   "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
   ", AccountNotLogin");
 } 
 Else If ( A_CurrentEquity() == InvalidNumeric )
 {
  FileAppend(LogPath,
   "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
   ", A_CurrentEquity Returns InvalidNumeric");
 }
 Else If ( A_TotalPosition() == InvalidNumeric )
 {
  FileAppend(LogPath,
   "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
   ", A_TotalPosition Returns InvalidNumeric");
 }
 Else If(Q_Last() == InvalidNumeric)
 {
  FileAppend(LogPath,
   "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
   ", Q_Last Returns InvalidNumeric");
 }
 Else
 {
  If(InvalidString != GetTBProfileString2File(ParamPath, SymbolType(), "EntryNumber"))
  {
   EntryNumber = Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryNumber"));
  }
 
  If(InvalidString != GetTBProfileString2File(ParamPath, SymbolType(), "PreEntryPrice"))
  {
   PreEntryPrice = Value(GetTBProfileString2File(ParamPath, SymbolType(), "PreEntryPrice"));
  }
 
  If ("True" == GetTBProfileString2File(ParamPath, SymbolType(), "PreBreakoutFailure"))
  {
   PreBreakoutFailure = True;
  }

  SetTBProfileString2File(ParamPath, SymbolType(), "TotalPosition", A_TotalPosition())
  SetTBProfileString2File(ParamPath, SymbolType(), "TotalRisk", Abs(A_TotalPosition()*N*ContractUnit()*BigPointValue())/A_CurrentEquity())
  
  TurtleUnits = (A_CurrentEquity() * RiskRatio /100) /(N * ContractUnit() * BigPointValue());
  TurtleUnits = IntPart(TurtleUnits); 
  ContractMargin = ContractUnit() * Close * MarginRatio();
  If (A_FreeMargin() < ContractMargin * TurtleUnits)
  {
   // Alert("No enough free margin, " + SymbolName());
   FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", No enough free margin") ;
   TurtleUnits = A_FreeMargin() / ContractMargin;
   TurtleUnits = IntPart(TurtleUnits);
  }
  If(SymbolType() == "I")
  {
    If (GetTBProfileString2File(ParamPath, "Pp", "TotalRisk")>0)
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On PP") ;
    }
  }
  Else If(SymbolType() == "Pp")
  {
    If (GetTBProfileString2File(ParamPath, "I", "TotalRisk")>0)
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On I") ;
    }
  }
  Else If(SymbolType() == "Y")
  {
    If ((GetTBProfileString2File(ParamPath, "M", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Rm", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On M and Rm") ;
    }
    Else If(GetTBProfileString2File(ParamPath, "M", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Oi", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On M and Oi") ;
    }
    Else If ((GetTBProfileString2File(ParamPath, "Oi", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Rm", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Oi and Rm") ;
    }
  }
  Else If(SymbolType() == "M")
  {
    If ((GetTBProfileString2File(ParamPath, "Y", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Rm", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Y and Rm") ;
    }
    Else If(GetTBProfileString2File(ParamPath, "Y", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Oi", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Y and Oi") ;
    }
    Else If ((GetTBProfileString2File(ParamPath, "Oi", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Rm", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Oi and Rm") ;
    }
  }
  Else If(SymbolType() == "Rm")
  {
    If ((GetTBProfileString2File(ParamPath, "Y", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "M", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Y and M") ;
    }
    Else If(GetTBProfileString2File(ParamPath, "Y", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Oi", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Y and Oi") ;
    }
    Else If ((GetTBProfileString2File(ParamPath, "Oi", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "M", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Oi and Y") ;
    }
  }
  Else If(SymbolType() == "Oi")
  {
    If ((GetTBProfileString2File(ParamPath, "Y", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "M", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Y and M") ;
    }
    Else If(GetTBProfileString2File(ParamPath, "Y", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "Rm", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Y and Rm") ;
    }
    Else If((GetTBProfileString2File(ParamPath, "Rm", "TotalRisk")>0)and(GetTBProfileString2File(ParamPath, "M", "TotalRisk")>0))
    {
      ClearToGo = False
      FileAppend(LogPath, 
    "DateTime = " + Text(CurrentDate() + CurrentTime()) + 
    ", Cannot Entry Because Position On Rm and Y") ;
    }
  }
// Check If there have open orders. Change instrument order state in setup file If have.
  If (InvalidInteger != A_GetOpenOrderCount() && 0 != A_GetOpenOrderCount())
  {
   ClearToGo = False;
   for i = 1 to A_GetOpenOrderCount()
   {
    If (Enum_Buy() == A_OpenOrderBuyOrSell(i))
    {
     TempOrderSide = "BUY";
    }
    Else If(Enum_Sell() == A_OpenOrderBuyOrSell(i))
    {
     TempOrderSide = "Sell";
    }
    FileAppend(LogPath, "HasOpenOrder_" + text(i) +
     ", OrderSide = " + TempOrderSide +
     ", OpenOrderContractNo = " + A_OpenOrderContractNo(i) +
     ", OpenOrderTime = " + Text(A_OpenOrderTime(i)) +
     ", OpenOrderLot = " + Text(A_OpenOrderLot(i)) +
     ", OpenOrderPrice = " + Text(A_OpenOrderPrice(i)) );
    for j = 1 to ReEntryLimit
    {
     If (Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryPrice_" + text(j))) == A_OpenOrderPrice(i)
      && Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryQuant_" + text(j))) == IIf(Enum_Buy() == A_OpenOrderBuyOrSell(i), A_OpenOrderLot(i), -1* A_OpenOrderLot(i)))
     {
      SetTBProfileString2File(ParamPath, SymbolType(), "EntryState_"+Text(j), "AcceptedNotFilled");
     }
     Break;
    }
    If (Abs(Close - A_OpenOrderPrice(i)) > MaxSlippage)
    {
     DeleteOrderSuccess = A_DeleteOrder(A_OpenOrderContractNo(i));
     FileAppend(LogPath, "DeleteOpenOrder" + text(i) +
      ", Result = " + IIfString(DeleteOrderSuccess,"True","False")); 
     for j = 1 to ReEntryLimit
     {
      If (Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryPrice_"+text(j))) == A_OpenOrderPrice(i)
       && Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryQuant_"+text(j))) == IIf(Enum_Buy() == A_OpenOrderBuyOrSell(i),A_OpenOrderLot(i), -1* A_OpenOrderLot(i)))
      {
       SetTBProfileString2File(ParamPath, SymbolType(), "EntryState_"+Text(j), "Deleted");
      }
      break;
     }
    }
   } 
  }
//Check If there are completed orders.
  If (InvalidInteger != A_GetOrderCount() && A_GetOrderCount()>0)
  {
   for i = 1 to A_GetOrderCount()
   {
    for j = 1 to ReEntryLimit
    {
    If (Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryPrice_" + text(j))) == A_OrderPrice(i)
     && Value(GetTBProfileString2File(ParamPath, SymbolType(), "EntryQuant_" + text(j))) == IIf(Enum_Buy() == A_OrderBuyOrSell(i), A_OrderFilledLot(i), -1* A_OrderFilledLot(i)))
    {
     SetTBProfileString2File(ParamPath, SymbolType(), "EntryState_"+Text(j), "Completed");
    }
    Break;
    }
   }
  }  
  
  for i = 1 to ReEntryLimit
  {
   If (GetTBProfileString2File(ParamPath, SymbolType(), "EntryState_" + text(i)) == "SentNotAccepted")
   {
    ClearToGo = False;
    Break;
   }
  }
   
  FileAppend(LogPath, 
   "DateTime = " + Text(CurrentDate() + CurrentTime()) +
   ", Open = " + Text(Open) +
   ", High = " + Text(High) +
   ", Low = " + Text(Low) +
   ", Q_Last = " + Text(Q_Last()) +
   ", Q_BidPrice = " + Text(Q_BidPrice()) +
   ", Q_AskPrice = " + Text(Q_AskPrice()) +
   ", N = " + Text(N) +
   ", TotalEquity = " + Text(A_CurrentEquity()) +
   ", TurtleUnits = " + Text(TurtleUnits) +
   ", DonchianHi = " + Text(DonchianHi) + 
   ", DonchianLo = " + Text(DonchianLo) + 
   ", fsDonchianHi = " + Text(fsDonchianHi) + 
   ", fsDonchianLo = " + Text(fsDonchianLo) + 
   ", ExitLowestPrice = " + Text(ExitLowestPrice) + 
   ", ExitHighestPrice = " + Text(ExitHighestPrice) + 
   ", A_AccountID = " + A_AccountID() +
   ", A_TotalPosition = " + Text(A_TotalPosition()) +
   ", A_TotalAvgPrice = " + Text(A_TotalAvgPrice()) +
   ", A_GetOpenOrderCount = " + Text(A_GetOpenOrderCount()) +
   ", EntryNumber = " + Text(EntryNumber) +
   ", PreEntryPrice = " + Text(PreEntryPrice) +
   ", ContractMargin = " + Text(ContractMargin) +
   ", PreBreakoutFailure = " + IIfString(PreBreakoutFailure, "True", "False") +
   ", ClearToGo = " + IIfString(ClearToGo, "True", "False"));
   
  If (ClearToGo)
  {
    //First Buy
   If ( EntryNumber == 0 && ( (PreBreakOutFailure && High > DonchianHi) || (High > fsDonchianHi) ) && TurtleUnits >= 1 )
   {
    OrderPrice = Q_AskPrice();
    EntryNumber = 1;
    PreBreakoutFailure = False;
    OrderSideMultiplier = 1;
    OrderQuantity = TurtleUnits;
    A_SendOrder(Enum_Buy, Enum_Entry, OrderQuantity, OrderPrice); 
   }
   //First Sell
   Else If ( EntryNumber == 0 && ((PreBreakOutFailure && Low < DonchianLo)||(Low < fsDonchianLo))&& TurtleUnits >= 1 )
   {
    OrderPrice = Q_BidPrice();
    EntryNumber = 1;
    PreBreakoutFailure = False;
    OrderSideMultiplier = -1;
    OrderQuantity = TurtleUnits;
    A_SendOrder(Enum_Sell, Enum_Entry, OrderQuantity, OrderPrice);
   }
// To Stoploss
   Else If ( A_TotalPosition() > 0 && Q_Last() <= PreEntryPrice - 2 * N && EntryNumber > 0 ) 
   {
    OrderPrice = Q_BidPrice();
    EntryNumber = 0;
    PreBreakoutFailure = True;
    OrderSideMultiplier = -1;
    OrderQuantity = Abs(A_TotalPosition());
    A_SendOrder(Enum_Sell, Enum_Exit, OrderQuantity, OrderPrice);
   }
// To Close Long Position
   Else If ( A_TotalPosition() > 0 && Q_Last() < ExitLowestPrice  && EntryNumber > 0 )
   {
    OrderPrice = Q_BidPrice(); 
    AvgCostPrice = (PreEntryPrice - (EntryTime -1)/2 * 0.5* N);
    If (OrderPrice < AvgCostPrice)
    {
     PreBreakoutFailure = True;
    }
    EntryNumber = 0;
    OrderSideMultiplier = -1;
    OrderQuantity = Abs(A_TotalPosition());
    A_SendOrder(Enum_Sell, Enum_Exit, OrderQuantity, OrderPrice); 
   }
// To Increase Long Position
   Else If ( A_TotalPosition() > 0 && Q_Last() >= PreEntryPrice + 0.5 * N && EntryNumber < ReEntryLimit && TurtleUnits >= 1 )
   {
    OrderPrice = Q_AskPrice();
    EntryNumber = EntryNumber + 1;
    OrderSideMultiplier = 1;
    OrderQuantity = TurtleUnits;
    A_SendOrder(Enum_Buy, Enum_Entry, OrderQuantity, OrderPrice); 
   } 
// To Stoploss   
   Else If ( A_TotalPosition() < 0 && Q_Last() >= PreEntryPrice + 2 * N && EntryNumber > 0 ) 
   {
    OrderPrice = Q_AskPrice();
    EntryNumber = 0;
    PreBreakoutFailure = True;
    OrderSideMultiplier = 1;
    OrderQuantity = Abs(A_TotalPosition());
    A_SendOrder(Enum_Buy, Enum_Exit, OrderQuantity, OrderPrice);
   }
// To Close Short Position
   Else If ( A_TotalPosition() < 0 && Q_Last() > ExitHighestPrice && EntryNumber > 0 )
   {
    OrderPrice = Q_AskPrice();
    AvgCostPrice = (PreEntryPrice + (EntryTime -1)/2 * 0.5* N);
    If (OrderPrice > AvgCostPrice)
    {
     PreBreakoutFailure = True;
    }
    EntryNumber = 0;
    OrderSideMultiplier = 1;
    OrderQuantity = Abs(A_TotalPosition());
    A_SendOrder(Enum_Buy, Enum_Exit, OrderQuantity, OrderPrice); 
   }
// To Increase Short Position
   Else If ( A_TotalPosition() < 0 && Q_Last() <= PreEntryPrice - 0.5 * N && EntryNumber < ReEntryLimit && TurtleUnits >= 1)
   {
    OrderPrice = Q_BidPrice();
    EntryNumber = EntryNumber + 1;
    OrderSideMultiplier = -1;
    OrderQuantity = TurtleUnits;
    A_SendOrder(Enum_Sell, Enum_Entry, OrderQuantity, OrderPrice); 
   }
// Save states
   SetTBProfileString2File(ParamPath, SymbolType(), "Symbol", Symbol());
   SetTBProfileString2File(ParamPath, SymbolType(), "EntryNumber", text(EntryNumber));
   SetTBProfileString2File(ParamPath, SymbolType(), "Current_N ", text(N));
   SetTBProfileString2File(ParamPath, SymbolType(), "PreBreakoutFailure", IIfString(PreBreakoutFailure, "True", "False"));
   // If There is a trade
   If (OrderSideMultiplier != 0)
   {
    If (EntryNumber > 0 )
    {
     If (OrderSideMultiplier == 1)
     {
      Alert("BUY "+SymbolName()+" "+Text(TurtleUnits)+" @ "+Text(OrderPrice)+", EntryTime = "+Text(EntryTime));
      FileAppend(LogPath, "BuyEntry" +
       ", OrderPrice = " + Text(OrderPrice) +
       ", Quantity = " + Text(OrderSideMultiplier * OrderQuantity) +
       ", EntryTimes = " + Text(EntryNumber)); 
     }
     Else If (OrderSideMultiplier == -1)
     {
      Alert("SELL "+SymbolName()+" "+Text(TurtleUnits)+" @ "+Text(OrderPrice)+", EntryTime = "+Text(EntryTime));
      FileAppend(LogPath, "SellEntry" +
       ", OrderPrice = " + Text(OrderPrice) +
       ", Quantity = " + Text(OrderSideMultiplier * OrderQuantity) +
       ", EntryTimes = " + Text(EntryNumber)); 
     }
     SetTBProfileString2File(ParamPath, SymbolType(), "PreEntryPrice", text(OrderPrice));
    }
    Else
    {
     Alert("ClosePos " + SymbolName() + " " + Text(OrderQuantity)+" @ "+Text(OrderPrice)+", EntryTime = "+Text(EntryTime));
     FileAppend(LogPath, "ClosePos" +
      ", OrderPrice = " + Text(OrderPrice));
    }
    SetTBProfileString2File(ParamPath, SymbolType(), "EntryQuant_"+Text(EntryNumber), Text(OrderSideMultiplier * OrderQuantity));
    SetTBProfileString2File(ParamPath, SymbolType(), "EntryPrice_"+Text(EntryNumber), Text(OrderPrice));
    SetTBProfileString2File(ParamPath, SymbolType(), "EntryState_"+Text(EntryNumber), "SentNotAccepted");
    SetTBProfileString2File(ParamPath, SymbolType(), "UpdateTime_"+Text(EntryNumber), Text(CurrentDate() + CurrentTime()));  
   }
   // End of "If there is a trade"
   TempEntryNumber = EntryNumber + 1;
   while(TempEntryNumber <= ReEntryLimit)
   {
    SetTBProfileString2File(ParamPath, SymbolType(), "EntryQuant_"+Text(TempEntryNumber), "0");
    SetTBProfileString2File(ParamPath, SymbolType(), "EntryPrice_"+Text(TempEntryNumber), "N/A");
    SetTBProfileString2File(ParamPath, SymbolType(), "EntryState_"+Text(TempEntryNumber), "N/A");
    SetTBProfileString2File(ParamPath, SymbolType(), "UpdateTime_"+Text(TempEntryNumber), "N/A");
    TempEntryNumber = TempEntryNumber + 1;
   }
  }
  //End of ClearToGo
 }
End
