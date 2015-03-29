#include <fstream>

#include <git2cpp/repo.h>
#include <git2cpp/initializer.h>

namespace
{
   struct visitor_t
   {
      void operator() (git::RevWalker & walker)
      {
         walker.sort(git::RevWalker::sorting::topological);
         walker.simplify_first_parent();

         while (git::Commit commit = walker.next())
         {
            output_commit(commit);

            for (size_t i = 0; i != commit.parents_num(); ++i)
            {
               output_hash(commit.id()) << " -> ";
               output_hash(commit.parent_id(i)) << "\n";
            }

            for (size_t i = 1; i < commit.parents_num(); ++i)
            {
               auto branch_walker = repo_.rev_walker();
               branch_walker.push(commit.parent_id(i));
               branch_walker.hide(commit.merge_base(0, i));
               (*this)(branch_walker);
            }
         }
      }

      visitor_t(git::Repository const & repo, std::ostream & out)
         : repo_(repo)
         , out_(out)
      {}

   private:
      std::ostream& output_hash(git_oid const & id) const
      {
         out_ << "C" << git::id_to_str(id, 6);
         return out_;
      }

      void output_commit(git::Commit const & commit) const
      {
         output_hash(commit.id()) << " [label=\"" << commit.summary() << "\"];" << "\n";
      }

   private:
      git::Repository const & repo_;
      std::ostream & out_;
   };

   void visit(git::Repository const & repo, std::ostream & out)
   {
      visitor_t visitor(repo, out);

      git::RevWalker walker = repo.rev_walker();
      walker.push_head();
      visitor(walker);
   }
}

int main(int argc, char * argv[])
{
   auto_git_initializer;
   git::Repository   repo  (argc >= 2 ? argv[1] : ".");
   std::ofstream     out   (argc >= 3 ? argv[2] : "commit-graph.dot");

   out << "digraph {\n";
   visit(repo, out);
   out << "}\n";
}
