defmodule ElixirStrategy.Mixfile do
  use Mix.Project

  @path if System.get_env("SOLUTION_CODE_PATH"), do: System.get_env("SOLUTION_CODE_PATH"), else: "."
  @name if System.get_env("SOLUTION_CODE_ENTRYPOINT"), do: System.get_env("SOLUTION_CODE_ENTRYPOINT"), else: "strategy"

  def project do
    [app: :elixir_strategy,
     version: "0.1.0",
     elixir: "~> 1.1",
     escript: [main_module: ElixirStrategy, path: @path <> "/" <> @name],
     deps: deps()]
  end

  defp deps do
    [{:poison, "~> 3.1"}]
  end
end
