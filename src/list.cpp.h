// Legacy Engine
// -------------
// list.cpp.h - Defines template list and stack classes (Template Implementation)
// -----------
// "©" David Scherfgen 2004, Terriermon 2005, Nameless 2007-2009

#ifndef LEGACY_LIST_CPP_H
#define LEGACY_LIST_CPP_H

// List Base Class
// ---------------
template <typename Type> BaseList<Type>::BaseList()
{
	pFirst = NULL;
	pLast = NULL;
	NumEntries = 0;
}

template <typename Type> BaseListEntry<Type>* BaseList<Type>::GetBaseEntry(ulong Index)
{
	BaseListEntry<Type>* pCurrentEntry = pFirst;
	if(Index > NumEntries)	return NULL;	// Out of Range

	for(ulong Count = 0; Count < Index; Count++)
	{
		pCurrentEntry = pCurrentEntry->pNextEntry;
		if(!pCurrentEntry)	return NULL;
	}
	return pCurrentEntry;
}

template <typename Type> BaseListEntry<Type>* BaseList<Type>::AppendEntry()
{
	BaseListEntry<Type>* pNewEntry = (BaseListEntry<Type>*)NewEntry();
	if(!pNewEntry)	return NULL;

	// Der neue Eintrag steht an letzter Stelle der Liste.
	// Daher gibt es keinen nächsten Eintrag und der vorherige Eintrag ist der ursprüngliche letzte Listeneintrag.
	pNewEntry->pPrevEntry = pLast;
	if(pLast) pLast->pNextEntry = pNewEntry;
	pNewEntry->pNextEntry = NULL;
	pLast = pNewEntry;

	if(!pFirst) pFirst = pNewEntry;	// Wenn die Liste noch ganz leer ist, dann ist dieser Eintrag der erste Eintrag

	NumEntries++;
	return pNewEntry;					// Zeiger auf die Listeneintragsstruktur zurückliefern
}

template <typename Type> BaseListEntry<Type>* BaseList<Type>::InsertEntry(BaseListEntry<Type>* pPrev)
{
	BaseListEntry<Type>* pNewEntry = (BaseListEntry<Type>*)NewEntry();
	if(!pNewEntry)	return NULL;

	if(!pPrev)
	{
		// The new entry is added to the beginning of the list.
		if(pFirst)
		{
			pNewEntry->pPrevEntry = NULL;
			pFirst->pPrevEntry = pNewEntry;
		}
		pNewEntry->pNextEntry = pFirst;
		pFirst = pNewEntry;
	}
	else
	{
		// If there is another entry behind the inserted one, the 
		// previous entry of this one has to be the inserted one.
		if(pPrev->pNextEntry)	pPrev->pNextEntry->pPrevEntry = pNewEntry;
		else					pLast = pNewEntry;

		pNewEntry->pNextEntry = pPrev->pNextEntry;
		pPrev->pNextEntry = pNewEntry;

		pNewEntry->pPrevEntry = pPrev;
	}

	NumEntries++;
	return pNewEntry;
}

// Delete list entry, returns next one
template <typename Type> BaseListEntry<Type>* BaseList<Type>::BaseDelete(BaseListEntry<Type>* pEntry)
{
	if(!pEntry) return NULL;
	
	BaseListEntry<Type>* r = pEntry->pNextEntry;

	// Beim Löschen entsteht ein Loch in der Liste, welches nun "gestopft" werden muss. Dabei spielt es eine Rolle, ob der Eintrag an erster
	// oder letzter Stelle oder irgendwo in der Mitte der Liste steht.
	if(pEntry == pFirst && pEntry == pLast)
	{
		// The entry is the first and only one
		pFirst = pLast = NULL;
		r = NULL;
	}
	else if(pEntry == pFirst)
	{
		// Der Eintrag steht an erster Stelle.
		// Der neue erste Eintrag ist nun der Folgende des zu löschenden Eintrags.
		pFirst = pEntry->pNextEntry;
		pFirst->pPrevEntry = NULL;
	}
	else if(pEntry == pLast)
	{
		// Der Eintrag steht an letzter Stelle.
		// Der neue letzte Eintrag ist nun der Vorherige des zu löschenden Eintrags.
		pLast = pEntry->pPrevEntry;
		pLast->pNextEntry = NULL;
	}
	else
	{
		// Der Eintrag steht irgendwo in der Mitte.
		// Der vorherige und der folgende Eintrag werden nun verknüpft.
		pEntry->pPrevEntry->pNextEntry = pEntry->pNextEntry;
		pEntry->pNextEntry->pPrevEntry = pEntry->pPrevEntry;
	}

	NumEntries--;
	return r;
}
// ---------------

// Normal List
// -----------
template <typename Type> List<Type>::~List()	{Clear();}

template <typename Type> BaseListEntry<Type>* List<Type>::NewEntry()
{
 	return new ListEntry<Type>;
}

template <typename Type> void List<Type>::SetEntryData(BaseListEntry<Type>* Entry, const Type* pData, ulong Reserved)
{
	ListEntry<Type>* pNewEntry = (ListEntry<Type>*)Entry;
	if(pData)	memcpy(&pNewEntry->Data, pData, sizeof(Type));
}

// Neuen Listeneintrag hinzufügen
template <typename Type> ListEntry<Type>* List<Type>::Add(const Type* pData)
{
	ListEntry<Type>* pNewEntry = (ListEntry<Type>*)BaseList<Type>::AppendEntry();
	if(!pNewEntry)	return NULL;

	SetEntryData(pNewEntry, pData);

	return pNewEntry;
}

// Neuen Listeneintrag einfügen
template <typename Type> ListEntry<Type>* List<Type>::Insert(ListEntry<Type>* pPrev, const Type* pData)
{
	ListEntry<Type>* pNewEntry =(ListEntry<Type>*)InsertEntry(pPrev);
	if(!pNewEntry)	return NULL;

	SetEntryData(pNewEntry, pData);	// Daten kopieren

	return pNewEntry;
}

// Sucht einen Eintrag in der Liste mit den angegebenen Daten
template <typename Type> ListEntry<Type>* List<Type>::Find(Type* pData)
{
	ListEntry<Type>* CurEntry = First();

	if(!pData) return 0;

	while(CurEntry)
	{
		// Die Daten des aktuellen Eintrags mit den angegebenen Daten vergleichen. Falls sie übereinstimmen, ist die Suche beendet.
		if(!memcmp(&CurEntry->Data, pData, sizeof(Type)))
		{
			return CurEntry;
		}
		CurEntry = CurEntry->Next();
	}

	// Es wurde nichts gefunden!
	return 0;
}

template <typename Type> ListEntry<Type>* List<Type>::Get(ulong Index)
{
	return (ListEntry<Type>*)BaseList<Type>::GetBaseEntry(Index);
}

template <typename Type> ListEntry<Type>* List<Type>::Delete(ListEntry<Type>* pEntry)
{
	ListEntry<Type>* r = (ListEntry<Type>*)BaseList<Type>::BaseDelete(pEntry);
	
	SAFE_DELETE(pEntry);
	return r;
}

// Setzt die Liste auf eine bestimmte Größe
template <typename Type> void List<Type>::Resize(ulong Size)
{
	ulong Count;
	ulong Entries = BaseList<Type>::NumEntries;	// Die Anzahl der Einträge verändert sich ja... ;-)

	if(Size > Entries)
	{
		for(Count = Entries; Count < Size; Count++)	Add(NULL);
	}
	else if(Size < Entries)
	{
		for(Count = Size; Count < Entries; Count++) Delete(Last());
	}
	return;
}

// Copy list
template <typename Type> bool List<Type>::Copy(List<Type>* Source, bool Append)
{
	if(!Source)	return false;
	if(!Append)	Clear();

	ListEntry<Type>* SrcEntry = Source->First();

	while(SrcEntry)
	{
		Add(&SrcEntry->Data);
		SrcEntry = SrcEntry->Next();
	}
	return true;
}

// Returns size of the list in memory
template <typename Type> ulong List<Type>::MemSize()
{
	return 2 * sizeof(ListEntry<Type>*) + sizeof(ulong) + First()->MemSize() * BaseList<Type>::NumEntries;
}

// Delete first entry
template <typename Type> ListEntry<Type>* List<Type>::Pop()
{
	return Delete(First());
}

// Gesamte Liste löschen
template <typename Type> void List<Type>::Clear()
{
	// Es wird so lange der erste Listeneintrag gelöscht, bis keiner mehr da ist
	while(First())	Pop();
}
// -----------

// Pointer List
// ------------
template <typename Type> PList<Type>::~PList()	{Clear();}

template <typename Type> BaseListEntry<Type>* PList<Type>::NewEntry()
{
	return new PListEntry<Type>;
}

template <typename Type> void PList<Type>::SetEntryData(BaseListEntry<Type>* Entry, const Type* pData, ulong Size)
{
	PListEntry<Type>*	pNewEntry = (PListEntry<Type>*)Entry;
	pNewEntry->Data = new Type[Size];
	memcpy(pNewEntry->Data, pData, sizeof(Type) * Size);
	pNewEntry->Size = Size;
}

// Add a new list entry
template <typename Type> PListEntry<Type>* PList<Type>::Add(const Type* pData, ulong Size)
{
	PListEntry<Type>* pNewEntry = (PListEntry<Type>*)BaseList<Type>::AppendEntry();
	if(!pNewEntry)	return NULL;

	SetEntryData(pNewEntry, pData, Size);

	return pNewEntry;
}

// Insert a new list entry at a given position
template <typename Type> PListEntry<Type>* PList<Type>::Insert(PListEntry<Type>* pPrev, const Type* pData, ulong Size)
{
	PListEntry<Type>* pNewEntry = (PListEntry<Type>*)InsertEntry(pPrev);
	if(!pNewEntry)	return NULL;

	SetEntryData(pNewEntry, pData, Size);

	return pNewEntry;
}

template <typename Type> PListEntry<Type>* PList<Type>::Find(Type* Data)
{
	if(!Data) return NULL;

	PListEntry<Type>* CurEntry = First();

	while(CurEntry)
	{
		if(!memcmp(&CurEntry->Data, Data, sizeof(Type) * CurEntry->Size))	return CurEntry;
		CurEntry = CurEntry->Next();
	}

	return NULL;
}

template <typename Type> PListEntry<Type>* PList<Type>::Get(ulong Index)
{
	return (PListEntry<Type>*)BaseList<Type>::GetBaseEntry(Index);
}

template <typename Type> PListEntry<Type>* PList<Type>::Delete(PListEntry<Type>* pEntry)
{
	PListEntry<Type>* r = (PListEntry<Type>*)BaseList<Type>::BaseDelete(pEntry);

	SAFE_DELETE(pEntry);
	return r;
}

// Delete first entry
template <typename Type> PListEntry<Type>* PList<Type>::Pop()
{
	return Delete(First());
}

// Copy list
template <typename Type> bool PList<Type>::Copy(PList<Type>* Source, bool Append)
{
	if(!Source)	return false;
	if(!Append)	Clear();

	PListEntry<Type>* SrcEntry = Source->First();

	while(SrcEntry)
	{
		Add(SrcEntry->Data, SrcEntry->Size);
		SrcEntry = SrcEntry->Next();
	}
	return true;
}

// Gesamte Liste löschen
template <typename Type> void PList<Type>::Clear()
{
	// Es wird so lange der erste Listeneintrag gelöscht, bis keiner mehr da ist
	while(First())	Pop();
}

// Returns size of the list in memory
template <typename Type> ulong PList<Type>::MemSize()
{
	ulong MS = 2 * sizeof(PListEntry<Type>*) + sizeof(ulong);
	PListEntry<Type>* CurEntry = First();
	while(CurEntry)
	{
		MS += CurEntry->MemSize();
		CurEntry = CurEntry->Next();
	}

	return MS;
}
// ------------

// Stack List
// ----------
template <typename Type> Stack<Type>::Stack(ulong Size)	{StackSize = Size;}

template <typename Type> ListEntry<Type>* Stack<Type>::Add(Type* pData)
{
	ListEntry<Type>* pNewEntry = (ListEntry<Type>*)List<Type>::AppendEntry();
	if(!pNewEntry)	return NULL;

	if(List<Type>::NumEntries == StackSize)	List<Type>::Delete(List<Type>::pFirst);

	SetEntryData(pNewEntry, pData);

	return pNewEntry;
}
// ----------

#endif /* LEGACY_LIST_CPP_H */
