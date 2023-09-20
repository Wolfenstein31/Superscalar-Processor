Superscalar Out-of-Order Pipeline Simulator 
Programmed the dynamic scheduling simulator mechanism for out-of-order pipeline super scalar processor that fetches and issues N-instructions per cycle. The program uses a Reorder buffer which makes sure that instructions are retired in order. 
Resolved WAR hazards in dataflow, WAW and RAW hazards using Register Renaming.
Eliminated false dependencies by implementing Reorder Buffer, issue queue and Register Renaming.
