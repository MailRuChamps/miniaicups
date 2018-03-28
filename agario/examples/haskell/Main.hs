{-# LANGUAGE NoImplicitPrelude, LambdaCase, OverloadedStrings #-}

module Main where

import ClassyPrelude
import Data.Aeson
import qualified System.IO as IO


data Coords = Coords Float Float


data Me = Me Coords

instance FromJSON Me where
  parseJSON = withObject "Me" $ \o ->
    Me <$> (Coords <$> o .: "X" <*> o .: "Y")


data ObjType = Food | OtherStuff Text

instance FromJSON ObjType where
  parseJSON = withText "Object Type" $ pure . \case
    "F" -> Food
    t   -> OtherStuff t

data Obj = Obj Coords ObjType

instance FromJSON Obj where
  parseJSON = withObject "Object" $ \o ->
    Obj <$> (Coords <$> o .: "X" <*> o .: "Y") <*> o .: "T"


data State = State [Me] [Obj]

instance FromJSON State where
  parseJSON = withObject "State" $ \o ->
    State <$> o .: "Mine" <*> o .: "Objects"


data Action = Skip Text | GoTo Coords

instance ToJSON Action where
  toJSON = \case
    Skip msg -> object
      [ "X"     .= (0 :: Float)
      , "Y"     .= (0 :: Float)
      , "Debug" .= msg
      ]
    GoTo (Coords x y) -> object ["X" .= x, "Y" .= y]


myStrategy :: State -> Action
myStrategy (State mines objs) =
  case (mines, [coords | Obj coords Food <- objs]) of
    ([], _)       -> Skip "Died"
    (_, [])       -> Skip "No food"
    (_, coords:_) -> GoTo coords

main :: IO ()
main = do
  IO.hSetBuffering stdin IO.LineBuffering
  IO.hSetBuffering stdout IO.LineBuffering

  _ <- unwrapErr . eitherDecodeStrict . encodeUtf8 <$> getLine :: IO Object
  interact $ unlines . fmap (decodeUtf8 . lbsCmd . encodeUtf8) . lines
  where
    unwrapErr = either error id
    lbsCmd = encode . myStrategy . unwrapErr . eitherDecode
