NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

INC_DIR = include
HEADER =	$(INC_DIR)/Channel.hpp \
			$(INC_DIR)/Client.hpp \
			$(INC_DIR)/Server.hpp

SRC =	src/main.cpp \
		src/Server.cpp

OBJ = $(SRC:.cpp=.o)

RM = rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER) Makefile
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re