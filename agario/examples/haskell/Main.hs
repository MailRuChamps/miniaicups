{-# LANGUAGE LambdaCase, OverloadedStrings #-}

module Main where

import Data.Aeson
import qualified Data.ByteString as BS
import qualified Data.ByteString.Lazy.Char8 as LBS
import Data.Text (Text)
import Control.Monad
import System.IO


data Coords = Coords Float Float


data Me = Me Coords

instance FromJSON Me where
  parseJSON = withObject "Me" $ \o ->
    Me <$> (Coords <$> o .: "X" <*> o .: "Y")


data ObjType = Food | OtherStuff Text

instance FromJSON ObjType where
  parseJSON = withText "Object Type" $ \case
    "F" -> pure Food
    t -> pure $ OtherStuff t

data Obj = Obj Coords ObjType

instance FromJSON Obj where
  parseJSON = withObject "Object" $ \o ->
    Obj <$> (Coords <$> o .: "X" <*> o .: "Y") <*> o .: "T"


data State = State [Me] [Obj]

instance FromJSON State where
  parseJSON = withObject "State" $ \o ->
    State <$> o .: "Mine" <*> o .: "Objects"


myStrategy :: State -> Value
myStrategy (State mines objs) =
  case mines of
    []   -> skip "Died"
    me:_ -> case [coords | Obj coords Food <- objs]  of
      []         -> skip "No food"
      (coords:_) -> goto coords
  where
    skip :: Text -> Value
    skip reason = object ["X" .= (0 :: Float), "Y" .= (0 :: Float), "Debug" .= reason]
    goto (Coords x y) = object ["X" .= x, "Y" .= y]


main :: IO ()
main = do
  hSetBuffering stdin LineBuffering
  hSetBuffering stdout LineBuffering

  cfg <- unwrapErr . eitherDecodeStrict <$> BS.getLine :: IO Object
  forever $
    unwrapErr . eitherDecodeStrict <$> BS.getLine >>=
      LBS.putStrLn . encode . myStrategy
  where
    unwrapErr = \case
      Right v -> v
      Left e  -> error e
