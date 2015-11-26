#include <array>

#include "git2cpp/id_to_str.h"

using namespace std;

string	git::id_to_str(git_oid const & id, const size_t digits_num)
{
	array<char, GIT_OID_HEXSZ + 1>	buf;
	
	git_oid_tostr(&buf[0], buf.size(), &id);
	
	const size_t	n = std::min(digits_num, (size_t) GIT_OID_HEXSZ);
	
	return string(buf.cbegin(), buf.cbegin() + n);
}

git_oid	git::str_to_id(const string &s)
{
	git_oid res;
	
	git_oid_fromstr(&res, s.c_str());
	
	return res;
}

git_oid git::str_to_id(const char *str)
{
	git_oid res;
	
	git_oid_fromstr(&res, str);
	
	return res;
}

