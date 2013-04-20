// Many thanks to http://mntone.hateblo.jp/entry/2013/02/13/115632

[Code]

procedure Pin(path: String; targets: Array of String);
var
  i,k: integer;
  itemName : String;
  FSO, folderPath, fileName, shell, folder, folderItem, colVerbs: Variant;
begin
  try
    FSO        := CreateOleObject('Scripting.FileSystemObject');
    folderPath := FSO.GetParentFolderName(path);
    fileName   := FSO.GetFileName(path);     

    shell      := CreateOleObject('Shell.Application');
    folder     := shell.NameSpace(folderPath);
    folderItem := folder.ParseName(fileName);

    colVerbs   := folderItem.Verbs;     
    for i := 0 to colVerbs.Count() do begin
      for k := 0 to High(targets) do begin
        itemName := colVerbs.Item(i).Name;
        StringChangeEx(itemName,'&','',false);
        if Pos(targets[k], itemName) > 0 then
          colVerbs.Item(i).DoIt();
      end;
    end;
  finally
  end;
end;


// Pin to StartMenu or Start
procedure PinToStartMenu(path: String);
var 
  win7, win8: Array of String;
begin 
  SetLength(win8, 2);
  win8[0] := 'Pin to Start';
  win8[1] := 'スタートにピン留め';
  SetLength(win7, 42);
  win7[0] := 'Pin to Start Menu';
  win7[1] := 'Fastgør til menuen Start';
  win7[2] := 'An Startmenü anheften';
  win7[3] := '&Καρφίτσωμα σε αυτήν τη λίστα';
  win7[4] := 'Anclar al menú Inicio';
  win7[5] := 'Kinnita menüüsse Start';
  win7[6] := 'Kiinnitä Käynnistä-valikkoon';
  win7[7] := 'Épingler au menu Démarrer';
  win7[8] := 'הצמד לתפריט התחלה';
  win7[9] := 'Prikvači na izbornik Start';
  win7[10] := 'Rögzítés a Start menün';
  win7[11] := 'Aggiungi al menu Start';
  win7[12] := 'Padaryti prieinamą meniu Pradėti';
  win7[13] := 'Piespraust izvēlnei Sākt';
  win7[14] := 'Fest til Start-menyen';
  win7[15] := 'Vastmaken aan menu Start';
  win7[16] := 'Przypnij do menu Start';
  win7[17] := 'Fixar no Menu Iniciar';
  win7[18] := 'Afixar no menu Iniciar';
  win7[19] := 'Fixare la meniul Start';
  win7[20] := 'Закрепить в меню Пуск';
  win7[21] := 'Pripnúť položku do ponuky Štart';
  win7[22] := 'Pripni v meni »Start«.';
  win7[23] := 'Dodaj u „Start“ meni';
  win7[24] := 'Fäst på Start-menyn';
  win7[25] := 'ตรึงกับเมนูเริ่ม';
  win7[26] := 'Başlat menüsüne sabitle';
  win7[27] := 'Прикріпити до меню Пуск';
  win7[28] := '시작 메뉴에 고정';
  win7[29] := 'スタート メニューに表示する';
  win7[30] := '锁定到「开始」菜单';
  win7[31] := '釘選到 [開始] 功能表';
  win7[32] := 'Закачи към менюто Старт';
  win7[33] := 'Připnout k nabídce Start';
  win7[34] := 'تثبيت بالقائمة ابدأ.';
  win7[35] := 'Pripni v meni »Start«';
  win7[36] := 'Įtraukti į meniu Pradėti';
  win7[37] := 'Pripnúť do ponuky Štart';
  win7[38] := 'Rögzítés a Start menüben';
  win7[39] := 'aan het menu Start Vastmaken';
  win7[40] := 'Odepnout k nabíce Start';
  win7[41] := 'Padaryti į meniu Pradėti'

  if IsSevenOrBefore() then
    Pin(path, win7)
end;

// Pin to Taskbar
procedure PinToTaskbar(path: String);
var 
  win7, win8: Array of String;
begin 
  SetLength(win7, 61);
  SetLength(win8, 2);
  win8[0] := 'Pin to Taskbar';
  win8[1] := 'タスク バーにピン留め';
  win7[0] := 'Pin to Taskbar';
  win7[1] := 'تثبيت هذا البرنامج إلى شريط المهام'; //アラビア語
  win7[2] := 'Закачи тази програма към лентата на задачите'; //ブルガリア
  win7[3] := 'Připnout tento program na hlavní panel'; //チェコ
  win7[4] := 'Fastgør programmet til proceslinjen'; //デンマーク
  win7[5] := 'Dieses Programm an Taskleiste anheften'; //ドイツ
  win7[6] := 'Καρφίτσωμα αυτού του προγράμματος στη γραμμή εργασιών';
  win7[7] := 'Anclar este programa a la barra de tareas'; //スペイン
  win7[8] := 'Kinnita see programm tegumiribale'; //エストニア
  win7[9] := 'Kiinnitä tämä ohjelma tehtäväpalkkiin'; //フィンランド
  win7[10] := 'Épingler ce programme à la barre des tâches'; //フランス
  win7[11] := 'הצמד תוכנית זו לשורת המשימות'; //ヘブライ
  win7[12] := 'Prikvači ovaj program na programsku traku'; //スロベニア
  win7[13] := 'A program rögzítése a tálcán'; //ハンガリー
  win7[14] := 'Aggiungi questo programma alla barra delle applicazioni'; //イタリア
  win7[15] := 'Šią programą padaryti prieinamą užduočių juostoje'; //リトアニア
  win7[16] := 'Piespraust šo programmu uzdevumjoslai'; //ラトビア
  win7[17] := 'Fest programmet til oppgavelinjen'; //ノルウェー
  win7[18] := 'Dit programma vastmaken aan de taakbalk'; //オランダ
  win7[19] := 'Przypnij ten program do paska zadań'; //ポーランド
  win7[20] := 'Fixar este programa na barra de tarefas';
  win7[21] := 'Afixar este programa na barra de tarefas';
  win7[22] := 'Fixare program la bara de activități';
  win7[23] := 'Закрепить программу в панели задач';
  win7[24] := 'Pripnúť tento program na panel úloh';
  win7[25] := 'Ta program pripni v opravilno vrstico';
  win7[26] := 'Zakači ovaj program na traku zadataka';
  win7[27] := 'Fäst det här programmet i Aktivitetsfältet';
  win7[28] := 'ตรึงโปรแกรมนี้เข้ากับแถบงาน';
  win7[29] := 'Bu programı görev çubuğuna sabitle';
  win7[30] := 'Прикріпити цю програму до панелі завдань';
  win7[31] := '이 프로그램을 작업 표시줄에 고정';
  win7[32] := 'タスク バーにこのプログラムを表示する';
  win7[33] := '将此程序从任务栏解锁';
  win7[34] := '將此程式釘選到工作列';
  win7[35] := 'Pin this program to taskbar';
  win7[36] := 'Görev çubuğuna sabitle';
  win7[37] := 'Aggiungi alla barra delle applicazioni';
  win7[38] := 'Épingler à la barre des tâches';
  win7[39] := 'Prikvači na programsku traku';
  win7[40] := 'Fäst i Aktivitetsfältet';
  win7[41] := 'Kinnita tegumiribale';
  win7[42] := 'Zakači na traku zadataka';
  win7[43] := 'Fastgør til proceslinje';
  win7[44] := 'pripni v opravilno vrstico';
  win7[45] := 'Pašalinti iš užduočių juostos';
  win7[46] := 'Įtraukti į užduočių juostą';
  win7[47] := 'Pripnúť na panel úloh';
  win7[48] := 'rögzítés a tálcán';
  win7[49] := 'Fixare la bara de activități';
  win7[50] := 'Afixar na barra de tarefas';
  win7[51] := 'Kiinnitä tehtäväpalkkiin';
  win7[52] := 'fixar na barra de tarefas';
  win7[53] := 'Anclar a la barra de tareas';
  win7[54] := 'Przypnij do paska zadań';
  win7[55] := 'an Taskleiste anheften';
  win7[56] := 'aan de taakbalk vastmaken';
  win7[57] := 'Připnout na hlavní panel';
  win7[58] := 'Fest til oppgavelinjen';
  win7[59] := 'Piespraust uzdevumjoslai';
  win7[60] := 'Įtraukti į užduočių juostą';
  if IsSevenOrLater() then begin
    if IsEightOrLater() then 
      Pin(path, win8)
    else     
      Pin(path, win8);
  end;
end;
