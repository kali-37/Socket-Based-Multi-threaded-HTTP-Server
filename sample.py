def is_walkable(map_relation: list[tuple], start: str, end: str) -> bool:
    hash_store = {}
    for map in map_relation:
        if not hash_store.get(map[0],[]):
            hash_store[map[0]]= []
        hash_store[map[0]].append(map[1])
    is_visited = []
    def dfs(source):
        if source == end:
            return True 
        is_visited.append(source)
        for i in hash_store.get(source,[]):
            if i !=[] and i not in is_visited:
                print("I",i)
                if dfs(i):
                    return True
        return False
    
    return dfs(start)


map_relation = [("A", "B"), ("A", "C"), ("B", "D"), ("D", "E")]
start = "A"
end = "E"
print(is_walkable(map_relation,start,end))