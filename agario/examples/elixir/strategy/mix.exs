defmodule Strategy.Mixfile do
  use Mix.Project

  @output_filename if System.get_env("COMPILED_FILE_PATH"), do: System.get_env("COMPILED_FILE_PATH"), else: "strategy"

  def project do
    [app: :elixir_strategy,
     version: "0.1.0",
     elixir: "~> 1.1",
     escript: [main_module: Strategy, path: @output_filename, emu_args: "+A 4 +S 4:4 +SDcpu 4:4"],
     deps: deps()]
  end

  defp deps do
    [{:poison, "~> 3.1"}]
  end
end
