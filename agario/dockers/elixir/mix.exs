defmodule ElixirStrategy.Mixfile do
  use Mix.Project

  def project do
    [app: :elixir_strategy,
     version: "0.1.0",
     elixir: "~> 1.1",
     escript: [main_module: ElixirStrategy],
     deps: deps()]
  end

  defp deps do
    [{:poison, "~> 3.1"}]
  end
end
