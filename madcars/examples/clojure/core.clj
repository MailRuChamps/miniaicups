(ns client.core
  (:require
    [cheshire.core :as json])
  (:gen-class))

(defn -main
  []
  (let [command-list ["left" "right" "stop"]]
    (loop []
      (let [in-line (read-line)]
        (when (not (empty? in-line))
          (let [in (json/parse-string in-line true)
                choice-com (nth command-list (rand-int 3))
                out (json/generate-string
                     {:command choice-com
                      :debug choice-com})]
            (println out))))
      (recur))))




