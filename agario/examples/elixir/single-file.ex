defmodule Strategy do
  def read_line(), do: :io.get_line(:standard_io, "")
  def write_line(line), do: IO.puts(line)

  def main(_) do
    line = read_line()
    {:ok, world_params} = Poison.decode(line)
    loop(%{world_params: world_params, state: 0})
  end

  def loop(state) do
    line = read_line()
    {:ok, world} = Poison.decode(line)
    case world["Mine"] do
      [mine|_] ->
        {resp, new_state} = world["Objects"]
        |> Enum.filter(fn(item) -> item["T"] == "F" end)
        |> case do
          [] ->
            cond do
              state.state == 0 and mine["X"]>=330 and mine["Y"]>=330 ->
                resp = %{"X" => 0, "Y" => 0}
                new_state = %{state | state: 1}
                {resp, new_state}
              state.state == 1 and mine["X"]<=50 and mine["Y"]<=50 ->
                resp = %{"X" => 330, "Y" => 330}
                new_state = %{state | state: 0}
                {resp, new_state}
              true ->
                case state.state do
                  0 ->
                    {resp = %{"X" => 330, "Y" => 330}, state}
                  1 ->
                    {resp = %{"X" => 0, "Y" => 0}, state}
                end
            end
          list ->
            item = hd(list)
            resp = %{"X" => item["X"], "Y" => item["Y"]}
            {resp, state}
        end
        {:ok, json} = Poison.encode(resp)
        write_line("#{json}")
        loop(new_state)
      _ ->
        # no "mine" object.
        write_line("{}")
        loop(state)
    end
  end
end