(defproject client "1.0.0"
  :main client.core
  :aot :all
  :uberjar-name "client.jar"
  :dependencies [[org.clojure/clojure "1.9.0"]
                 [cheshire "5.8.0"]
                 [org.clojure/core.async "0.4.474"]])
