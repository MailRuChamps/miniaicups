{-# LANGUAGE OverloadedStrings #-}

module Main where

import Data.Aeson
import System.IO
import Control.Monad
import Data.Maybe
import Data.ByteString.Lazy.Char8 as Char8
import System.Random

data Action = Action String String
data Tick = Tick String


instance FromJSON Tick where
  parseJSON = withObject "Tick" $ \ o -> Tick <$> o .: "type"

instance ToJSON Action where
  toJSON (Action command debug) = object ["command" .= command, "debug" .= debug]

takeAction :: Int -> String
takeAction 0 = "left"
takeAction 1 = "right"
takeAction 2 = "up"
takeAction 3 = "down"

myStrategy :: IO()
myStrategy = do
  maybeTick <- getLine >>= pure . decode . Char8.pack :: IO (Maybe Tick)
  gen <- newStdGen
  let defaultTick = Tick ""
  let action = takeAction $ fst $ (randomR (0, 3) gen :: (Int, StdGen))

  Char8.putStrLn $ encode (Action action action)

main :: IO ()
main = do
  hSetBuffering stdin LineBuffering
  hSetBuffering stdout LineBuffering
  forever myStrategy
