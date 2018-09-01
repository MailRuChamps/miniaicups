program main;

const
  COMMANDS: array[0..2] of string = (
    'left', 'right', 'stop'
    );

var
  Z: string;
  Cmd: string;

begin
  while True do
  begin
    if not Eof(input) then
    begin
      ReadLn(input, Z);
      Cmd := COMMANDS[Random(2)];
      WriteLn(output, '{"command": "' + Cmd + '", "debug": "' + Cmd + '"}');
    end;
  end;
end.